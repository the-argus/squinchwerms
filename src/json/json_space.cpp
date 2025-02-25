#include "json/json_space.h"
#include "debug_draw.h"
#include "natural_log/natural_log.h"
#include "space.h"
#include <okay/containers/array.h>
#include <okay/defer.h>
#include <okay/macros/foreach.h>
#include <okay/short_arithmetic_types.h>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <raylib.h>

using docmem_t = char *;

static docmem_t documentParse(rapidjson::Document &doc,
                              const char *filepath) noexcept
{
    if (!IsFileNameValid(filepath) || !FileExists(filepath)) {
        LN_ERROR_FMT("Invalid / missing file: {}", filepath);
        return nullptr;
    }

    char *text = LoadFileText(filepath);
    if (!text) {
        LN_ERROR_FMT("Failed to load file {}", text);
        return nullptr;
    }

    // modifies the read memory and avoids allocating more where possible
    doc.ParseInsitu(text);
    if (doc.HasParseError()) {
        UnloadFileText(text);
        LN_ERROR_FMT("Parse error {} trying to parse file {}",
                     int(doc.GetParseError()), filepath);
        return nullptr;
    }
    return text;
}

static bool serializeVersion(const lib::SerializationVersion &version,
                             rapidjson::Document &doc)
{
    doc.StartObject();

    const char *key = "major";
    doc.String(key, strlen(key), false);
    doc.Uint64(version.maj);
    key = "minor";
    doc.String(key, strlen(key), false);
    doc.Uint64(version.min);
    key = "patch";
    doc.String(key, strlen(key), false);
    doc.Uint64(version.patch);

    doc.EndObject(3);
    return true;
}

static ok::opt_t<lib::SerializationVersion>
deserializeVersion(rapidjson::Value &versionBlock)
{
#define opt_schemacheck(cond)                                   \
    if (!(cond)) {                                              \
        LN_ERROR_FMT("Bad JSON schema, {} was false", (#cond)); \
        return ok::nullopt;                                     \
    }
    opt_schemacheck(versionBlock.HasMember("major"));
    opt_schemacheck(versionBlock.HasMember("minor"));
    opt_schemacheck(versionBlock.HasMember("patch"));
    auto &maj = versionBlock["major"];
    auto &min = versionBlock["minor"];
    auto &patch = versionBlock["patch"];
    opt_schemacheck(maj.IsUint64());
    opt_schemacheck(min.IsUint64());
    opt_schemacheck(patch.IsUint64());
    return lib::SerializationVersion{
        .maj = maj.GetUint64(),
        .min = min.GetUint64(),
        .patch = patch.GetUint64(),
    };
}

static ok::opt_t<lib::Vect> deserializeVect(rapidjson::Value &json)
{
    opt_schemacheck(json.IsArray());
    using namespace rapidjson;
    u64 index = 0;
    lib::Vect out;
    for (Value &v : json.GetArray()) {
        opt_schemacheck(v.IsFloat());
        opt_schemacheck(index < 2);
        switch (index) {
        case 0:
            out.x = v.GetFloat();
            break;
        case 1:
            out.y = v.GetFloat();
            break;
        }
        ++index;
    }
    return out;
}

static ok::opt_t<lib::Shape &>
deserializeShape(rapidjson::Value &json, lib::Body &connectedBody,
                 const lib::PhysicsAllocators &allocators)
{
    using namespace rapidjson;
    opt_schemacheck(json.HasMember("type"));
    Value &typeref = json["type"];
    int rawType = typeref.GetInt();
    opt_schemacheck(rawType == u8(lib::Shape::Type::Poly) ||
                    rawType == u8(lib::Shape::Type::Segment));
    auto type = lib::Shape::Type(u8(rawType));

    switch (type) {
    case lib::Shape::Type::Poly: {
        opt_schemacheck(json.HasMember("vertices"));
        opt_schemacheck(json.HasMember("radius"));

        Value &verticesRef = json["vertices"];
        Value &radiusRef = json["radius"];

        opt_schemacheck(verticesRef.IsArray());
        opt_schemacheck(radiusRef.IsFloat());

        const u64 numVertices = verticesRef.GetArray().Size();
        const u64 bytesNeededExactly = sizeof(cpVect) * numVertices;

        // avoid overflow, could only happen if somebody edited the save file
        opt_schemacheck(numVertices < std::numeric_limits<int>::max());

        auto vertexBuf =
            allocators.vertexBufScratch.allocate(ok::alloc::request_t{
                .num_bytes = bytesNeededExactly,
                .alignment = alignof(cpVect),
            });

        if (!vertexBuf.okay()) {
            LN_ERROR("Failed to allocate space for physics vertex buffer "
                     "during deserialization");
            return ok::nullopt;
        }

        ok::slice_t<cpVect> allocatedVertices =
            ok::reinterpret_bytes_as<cpVect>(
                vertexBuf.release().as_bytes().subslice(
                    {.length = bytesNeededExactly}));
        ok::defer_t freeVertices([&] {
            allocators.vertexBufScratch.deallocate(
                ok::reinterpret_as_bytes(allocatedVertices));
        });

        size_t index = 0;
        ok_foreach(cpVect & vector, allocatedVertices)
        {
            if (auto attempt = deserializeVect(verticesRef.GetArray()[index])) {
                vector = attempt.value();
            } else {
                return ok::nullopt;
            }
            ++index;
        }
        assert(index == verticesRef.GetArray().Size());

        lib::PolyShape *poly = allocators.polyShapeAllocator();

        // poly shape does interal allocation and copying of our vertices
        cpPolyShapeInit(poly, &connectedBody, allocatedVertices.size(),
                        allocatedVertices.data(), cpTransformIdentity,
                        radiusRef.GetFloat());
        poly->shape.userData =
            lib::genMeshFromVertices(allocatedVertices, poly->radius(), RED);
        return poly->asShape();
        break;
    }
    case lib::Shape::Type::Segment: {
        opt_schemacheck(json.HasMember("a"));
        opt_schemacheck(json.HasMember("b"));
        opt_schemacheck(json.HasMember("radius"));
        Value &aref = json["a"];
        Value &bref = json["b"];
        Value &radiusref = json["radius"];
        opt_schemacheck(radiusref.IsFloat());
        lib::SegmentShape::Options options{
            .radius = radiusref.GetFloat(),
        };

        if (auto vect = deserializeVect(aref)) {
            options.a = vect.value();
        } else {
            return ok::nullopt;
        }

        if (auto vect = deserializeVect(bref)) {
            options.b = vect.value();
        } else {
            return ok::nullopt;
        }
        lib::SegmentShape *segment = allocators.segmentShapeAllocator();
        new (segment) lib::SegmentShape(connectedBody, options);
        return *segment->asShape();
    }
    default: {
        assert(false);
        __builtin_unreachable();
    }
    }
}

static ok::opt_t<lib::Body &>
deserializeBody(rapidjson::Value &json,
                const lib::PhysicsAllocators &allocators)
{
    using namespace rapidjson;
    opt_schemacheck(json.HasMember("mass"));
    opt_schemacheck(json.HasMember("moment"));
    opt_schemacheck(json.HasMember("type"));
    opt_schemacheck(json.HasMember("shapes"));

    Value &mass = json["mass"];
    Value &moment = json["moment"];
    Value &type = json["type"];
    Value &shapes = json["shapes"];
    opt_schemacheck(mass.IsFloat());
    opt_schemacheck(moment.IsFloat());
    opt_schemacheck(type.IsInt());
    opt_schemacheck(shapes.IsArray());

    lib::Body *uninitialized = allocators.bodyAllocator();
    int realType = type.GetInt();
    opt_schemacheck(realType == u8(lib::Body::Type::DYNAMIC) ||
                    realType == u8(lib::Body::Type::KINEMATIC) ||
                    realType == u8(lib::Body::Type::STATIC));
    new (uninitialized) lib::Body(lib::Body::BodyOptions{
        .type = lib::Body::Type(realType),
        .mass = mass.GetFloat(),
        .moment = moment.GetFloat(),
    });

    cpShape *shapeIter = nullptr;
    for (Value &shapeJson : shapes.GetArray()) {
        if (auto shape =
                deserializeShape(shapeJson, *uninitialized, allocators)) {
            if (!shapeIter) {
                shapeIter = &shape.value();
                uninitialized->shapeList = shapeIter;
            } else {
                shapeIter->next = &shape.value();
                shapeIter = &shape.value();
            }
        } else {
            return ok::nullopt;
        }
    }

    return uninitialized;
}

bool lib::deserializeSpace(Space &uninitialized, const char *filepath,
                           const PhysicsAllocators &allocators) noexcept
{
    using namespace rapidjson;
    Document doc;

    docmem_t insituMemory = documentParse(doc, filepath);
    ok::defer_t freeInsitu([insituMemory] { UnloadFileText(insituMemory); });

    if (!insituMemory) {
        return false;
    }

#define schemacheck(cond)                                      \
    if (!(cond)) {                                             \
        LN_ERROR_FMT("invalid schema, {} was false", (#cond)); \
        return false;                                          \
    }

    schemacheck(doc.IsObject());
    auto root = doc.GetObject();

    schemacheck(root.HasMember("version"));
    schemacheck(root.HasMember("bodies"));

    // get version
    {
        if (auto maybe_version = deserializeVersion(root["version"])) {
            lib::SerializationVersion &version = maybe_version.value();
            LN_INFO_FMT("loading file {} with schema version {}.{}.{}",
                        filepath, version.maj, version.min, version.patch);
        } else {
            return false;
        }
    }

    cpSpaceInit(&uninitialized);
    ok::maydefer_t freeSpace([&] { cpSpaceDestroy(&uninitialized); });

    Value &bodies = root["bodies"];
    schemacheck(bodies.IsArray());

    for (Value &item : bodies.GetArray()) {
        if (auto body = deserializeBody(item, allocators)) {
            uninitialized.add(body.value());
        }
    }

    freeSpace.cancel();
    return true;
}
