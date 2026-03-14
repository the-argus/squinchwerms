module;

#include "macros.h"
#include <box2d/box2d.h>

export module box2d;
import aliases;
import allocator;
import slice;
import res;

/// Helper for calling GetCapacity() -> Get*(T* buf, int capacity) functions
/// from box2d
template <typename T, typename IDType, typename GetCapacityCallable,
          typename GetBufferCallable>
[[nodiscard]] Res<Slice<T>, alloc::Error>
getDataBufferHelper(Allocator &allocator, IDType id,
                    const GetCapacityCallable &getCapacity,
                    const GetBufferCallable &getBuffer)
{
    const u32 needed = getCapacity(id);
    const auto buffer = allocator.allocate(alloc::Request{
        .numBytes = sizeof(T) * needed,
        .alignment = alignof(T),
    });
    if (not isSuccess(buffer)) [[unlikely]]
        return buffer.error();

    auto *const start =
        reinterpret_cast<T *>(buffer->uncheckedAddressOfFirstItem());
    w_assert(buffer->sizeInBytes() >= sizeof(T) * needed, "box2d bug");

    const int used = getBuffer(id, start, needed);
    w_assert(needed == static_cast<u32>(used), "box2d bug");

    return unsafe::rawSlice(*start, needed);
}

export namespace b2 {
using Vec2 = b2Vec2;
struct Vec2I32
{
    i32 x = 0;
    i32 y = 0;
};
struct Vec2U32
{
    u32 x = 0;
    u32 y = 0;
};
using Segment = b2Segment;
using Capsule = b2Capsule;
using Circle = b2Circle;
using AABB = b2AABB;
using ContactData = b2ContactData;
using MassData = b2MassData;
using Transform = b2Transform;
using Rotation = b2Rot;

constexpr Rotation rotationIdentity = {1.0f, 0.0f};

enum class BodyType
{
    /// zero mass, zero velocity, may be manually moved
    Static = 0,
    /// zero mass, velocity set by user, moved by solver
    Kinematic = 1,
    /// positive mass, velocity determined by forces, moved by solver
    Dynamic = 2,
};

using BodyID = b2BodyId;
using ShapeID = b2ShapeId;
using WorldID = b2WorldId;
using JointID = b2JointId;

struct BodyDef : public b2BodyDef
{
    BodyDef() NOEXCEPT : b2BodyDef(b2DefaultBodyDef()) {}
    explicit BodyDef(BodyType type) NOEXCEPT : b2BodyDef(b2DefaultBodyDef())
    {
        this->type = static_cast<b2BodyType>(type);
    }
    BodyDef(const b2BodyDef &actual) NOEXCEPT : b2BodyDef(actual) {}
};

struct ShapeDef : public b2ShapeDef
{
    ShapeDef() NOEXCEPT : b2ShapeDef(b2DefaultShapeDef()) {}
    ShapeDef(const b2ShapeDef &actual) NOEXCEPT : b2ShapeDef(actual) {}
};

struct WorldDef : public b2WorldDef
{
    WorldDef() NOEXCEPT : b2WorldDef(b2DefaultWorldDef()) {}
    WorldDef(const b2WorldDef &actual) NOEXCEPT : b2WorldDef(actual) {}
};

[[nodiscard]] BodyID createBody(WorldID world, const BodyDef &def) NOEXCEPT
{
    return b2CreateBody(world, &def);
}

[[nodiscard]] ShapeID createSegmentShape(BodyID body,
                                         const ShapeDef &definition,
                                         const Segment &segment) NOEXCEPT
{
    return b2CreateSegmentShape(body, &definition, &segment);
}

[[nodiscard]] ShapeID createCapsuleShape(BodyID body,
                                         const ShapeDef &definition,
                                         const Capsule &capsule) NOEXCEPT
{
    return b2CreateCapsuleShape(body, &definition, &capsule);
}

[[nodiscard]] ShapeID b2CreateCircleShape(BodyID body,
                                          const ShapeDef &definition,
                                          const Circle &circle) NOEXCEPT
{
    return b2CreateCircleShape(body, &definition, &circle);
}

[[nodiscard]] WorldID createWorld(const WorldDef &definition) NOEXCEPT
{
    return b2CreateWorld(&definition);
}

void destroyBody(BodyID body) NOEXCEPT { b2DestroyBody(body); }
void destroyShape(ShapeID shape, bool updateBodyMass) NOEXCEPT
{
    b2DestroyShape(shape, updateBodyMass);
}
void destroyWorld(WorldID world) NOEXCEPT { destroyWorld(world); }

using CastResultFunction = b2CastResultFcn;
using CustomFilterFunction = b2CustomFilterFcn;
using EnqueueTaskCallback = b2EnqueueTaskCallback;
using FinishTaskCallback = b2FinishTaskCallback;
using FrictionCallback = b2FrictionCallback;
using OverlapResultFunction = b2OverlapResultFcn;
using PreSolveFunction = b2PreSolveFcn;
using RestitutionCallback = b2RestitutionCallback;
using TaskCallback = b2TaskCallback;
using QueryFilter = b2QueryFilter;
using TreeStats = b2TreeStats;
using RayResult = b2RayResult;
using ShapeProxy = b2ShapeProxy;
using ExplosionDef = b2ExplosionDef;

[[nodiscard]] f32 worldCastMover(WorldID worldId, const Capsule &mover,
                                 Vec2 translation, QueryFilter filter) NOEXCEPT
{
    return b2World_CastMover(worldId, &mover, translation, filter);
}

[[nodiscard]] TreeStats worldCastRay(WorldID worldId, Vec2 origin,
                                     Vec2 translation, QueryFilter filter,
                                     CastResultFunction *fcn,
                                     void *context) NOEXCEPT
{
    return b2World_CastRay(worldId, origin, translation, filter, fcn, context);
}

[[nodiscard]] RayResult worldCastRayClosest(WorldID worldId, Vec2 origin,
                                            Vec2 translation,
                                            QueryFilter filter) NOEXCEPT
{
    return b2World_CastRayClosest(worldId, origin, translation, filter);
}

[[nodiscard]] TreeStats worldCastShape(WorldID worldId, const ShapeProxy &proxy,
                                       Vec2 translation, QueryFilter filter,
                                       CastResultFunction *function,
                                       void *context) NOEXCEPT
{
    return b2World_CastShape(worldId, &proxy, translation, filter, function,
                             context);
}

void worldEnableContinuous(WorldID worldId, bool enabled) NOEXCEPT
{
    b2World_EnableContinuous(worldId, enabled);
}

void worldEnableSleeping(WorldID worldId, bool enabled) NOEXCEPT
{
    b2World_EnableSleeping(worldId, enabled);
}

// /// Enable/disable constraint warm starting. Advanced feature for testing.
// /// Disabling warm starting greatly reduces stability and provides no
// /// performance gain.
// void worldEnableWarmStarting(WorldID worldId, bool enabled) NOEXCEPT
// {
//     b2World_EnableWarmStarting(worldId, enabled);
// }

/// Apply a radial explosion
/// @param worldId The world id
/// @param explosionDef The explosion definition
// void worldExplode(WorldID worldId, const ExplosionDef &explosionDef) NOEXCEPT
// {
//     b2World_Explode(worldId, &explosionDef);
// }

struct ContactTuningOptions
{
    float hertz;
    float dampingRatio;
    float pushSpeed;
};

void worldSetContactTuning(WorldID worldId,
                           const ContactTuningOptions &tuning) NOEXCEPT
{
    b2World_SetContactTuning(worldId, tuning.hertz, tuning.dampingRatio,
                             tuning.pushSpeed);
}

/// Set the gravity vector for the entire world. Box2D has no concept of an up
/// direction and this is left as a decision for the application. Usually in
/// m/s^2.
/// @see b2WorldDef
void worldSetGravity(WorldID worldId, Vec2 gravity) NOEXCEPT
{
    b2World_SetGravity(worldId, gravity);
}

/// Adjust the hit event threshold. This controls the collision speed needed to
/// generate a b2ContactHitEvent. Usually in meters per second.
/// @see b2WorldDef::hitEventThreshold
void worldSetHitEventThreshold(WorldID worldId, f32 value) NOEXCEPT
{
    b2World_SetHitEventThreshold(worldId, value);
}

// struct JointTuningOptions
// {
// 	float hertz;
// 	float dampingRatio;
// };
// void worldSetJointTuning 	( 	WorldID 	worldId, const
// JointTuningOptions& options ) NOEXCEPT { 	b2World_SetJointTuning(worldId,
// options.hertz, options.dampingRatio);
// }

void worldSetRestitutionThreshold(WorldID worldId, f32 value) NOEXCEPT
{
    b2World_SetRestitutionThreshold(worldId, value);
}

void worldStep(WorldID worldId, f32 timeStep, u16 subStepCount = 4) NOEXCEPT
{
    b2World_Step(worldId, timeStep, static_cast<int>(subStepCount));
}

// body --------------------------

void bodyApplyAngularImpulse(BodyID bodyId, f32 impulse, bool wake) NOEXCEPT
{
    b2Body_ApplyAngularImpulse(bodyId, impulse, wake);
}

void bodyApplyForce(BodyID bodyId, Vec2 force, Vec2 point, bool wake) NOEXCEPT
{
    b2Body_ApplyForce(bodyId, force, point, wake);
}

void bodyApplyForceToCenter(BodyID bodyId, Vec2 force, bool wake) NOEXCEPT
{
    b2Body_ApplyForceToCenter(bodyId, force, wake);
}

void bodyApplyLinearImpulse(BodyID bodyId, Vec2 impulse, Vec2 point,
                            bool wake) NOEXCEPT
{
    b2Body_ApplyLinearImpulse(bodyId, impulse, point, wake);
}

void bodyApplyLinearImpulseToCenter(BodyID bodyId, Vec2 impulse,
                                    bool wake) NOEXCEPT
{
    b2Body_ApplyLinearImpulseToCenter(bodyId, impulse, wake);
}

void bodyApplyMassFromShapes(BodyID bodyId) NOEXCEPT
{
    b2Body_ApplyMassFromShapes(bodyId);
}

void bodyApplyTorque(BodyID bodyId, float torque, bool wake) NOEXCEPT
{
    b2Body_ApplyTorque(bodyId, torque, wake);
}

[[nodiscard]] AABB bodyComputeAABB(BodyID bodyId) NOEXCEPT
{
    return b2Body_ComputeAABB(bodyId);
}

void bodyEnableContactEvents(BodyID bodyId, bool isEnabled) NOEXCEPT
{
    b2Body_EnableContactEvents(bodyId, isEnabled);
}

void bodyEnableHitEvents(BodyID bodyId, bool isEnabled) NOEXCEPT
{
    b2Body_EnableHitEvents(bodyId, isEnabled);
}

/// Get the maximum capacity required for retrieving all the touching contacts
/// on a body
[[nodiscard]] u32 bodyGetContactCapacity(BodyID bodyId) NOEXCEPT
{
    return static_cast<u32>(b2Body_GetContactCapacity(bodyId));
}

/// Get the touching contact data for a body.
/// @note Box2D uses speculative collision so some contact points may be
/// separated.
/// @returns the number of elements filled in the provided array
/// @warning do not ignore the return value, it specifies the valid number of
/// elements
[[nodiscard]] int bodyGetContactDataUnsafe(BodyID bodyId,
                                           ContactData *contactData,
                                           u32 capacity) NOEXCEPT
{
    return b2Body_GetContactData(bodyId, contactData,
                                 static_cast<int>(capacity));
}

[[nodiscard]] Res<Slice<ContactData>, alloc::Error>
bodyGetContactData(Allocator &allocator, BodyID bodyId) NOEXCEPT
{
    return getDataBufferHelper<ContactData>(
        allocator, bodyId, bodyGetContactCapacity, bodyGetContactDataUnsafe);
}

[[nodiscard]] u32 bodyGetJointCount(BodyID bodyId) NOEXCEPT
{
    return static_cast<u32>(b2Body_GetJointCount(bodyId));
}

[[nodiscard]] int bodyGetJointsUnsafe(BodyID bodyId, JointID *jointIdBuffer,
                                      u32 capacity) NOEXCEPT
{
    return b2Body_GetJoints(bodyId, jointIdBuffer, static_cast<int>(capacity));
}

[[nodiscard]] Res<Slice<JointID>, alloc::Error>
bodyGetJoints(Allocator &allocator, BodyID bodyId) NOEXCEPT
{
    return getDataBufferHelper<JointID>(allocator, bodyId, bodyGetJointCount,
                                        bodyGetJointsUnsafe);
}

[[nodiscard]] u32 bodyGetShapeCount(BodyID bodyId) NOEXCEPT
{
    return static_cast<u32>(b2Body_GetShapeCount(bodyId));
}

[[nodiscard]] int bodyGetShapesUnsafe(BodyID bodyId, ShapeID *shapeIdBuffer,
                                      u32 capacity) NOEXCEPT
{
    return b2Body_GetShapes(bodyId, shapeIdBuffer, static_cast<int>(capacity));
}

[[nodiscard]] Res<Slice<ShapeID>, alloc::Error>
bodyGetShapes(Allocator &allocator, BodyID bodyId) NOEXCEPT
{
    return getDataBufferHelper<ShapeID>(allocator, bodyId, bodyGetShapeCount,
                                        bodyGetShapesUnsafe);
}

void bodySetAwake(BodyID body, bool newIsAwake) NOEXCEPT
{
    return b2Body_SetAwake(body, newIsAwake);
}

[[nodiscard]] bool bodyIsAwake(BodyID body) NOEXCEPT
{
    return b2Body_IsAwake(body);
}

void bodySetBullet(BodyID body, bool newIsBullet) NOEXCEPT
{
    b2Body_SetBullet(body, newIsBullet);
}

[[nodiscard]] bool bodyIsBullet(BodyID body) NOEXCEPT
{
    return b2Body_IsBullet(body);
}

void bodySetGravityScale(BodyID body, f32 newGravityScale) NOEXCEPT
{
    b2Body_SetGravityScale(body, newGravityScale);
}

[[nodiscard]] f32 bodyGetGravityScale(BodyID body) NOEXCEPT
{
    return b2Body_GetGravityScale(body);
}

void bodySetMassData(BodyID body, const MassData &newMassData) NOEXCEPT
{
    b2Body_SetMassData(body, newMassData);
}

[[nodiscard]] MassData bodyGetMassData(BodyID body) NOEXCEPT
{
    return b2Body_GetMassData(body);
}

void bodySetTargetTransform(BodyID body, Transform transform,
                            f32 timeStep) NOEXCEPT
{
    b2Body_SetTargetTransform(body, transform, timeStep);
}

void bodySetTransform(BodyID body, Vec2 position, Rotation rotation) NOEXCEPT
{
    b2Body_SetTransform(body, position, rotation);
}

Vec2 bodyGetPosition(BodyID bodyId) { return b2Body_GetPosition(bodyId); }

Rotation bodyGetRotation(BodyID bodyId) { return b2Body_GetRotation(bodyId); }

[[nodiscard]] Transform bodyGetTransform(BodyID body) NOEXCEPT
{
    return b2Body_GetTransform(body);
}

void bodySetType(BodyID body, BodyType newType) NOEXCEPT
{
    b2Body_SetType(body, static_cast<b2BodyType>(newType));
}

[[nodiscard]] BodyType bodyGetType(BodyID bodyId) NOEXCEPT
{
    return static_cast<BodyType>(b2Body_GetType(bodyId));
}

[[nodiscard]] bool bodyIsValid(BodyID body) NOEXCEPT
{
    return b2Body_IsValid(body);
}

void bodySetName(BodyID bodyId, const char *name) NOEXCEPT
{
    b2Body_SetName(bodyId, name);
}

const char *bodyGetName(BodyID bodyId)
{
    auto *out = b2Body_GetName(bodyId);
    return out ? out : "UNKNOWNBODYNAME";
}

void bodySetUserData(BodyID bodyId, void *userData)
{
    b2Body_SetUserData(bodyId, userData);
}

void *bodyGetUserData(BodyID bodyId) NOEXCEPT
{
    return b2Body_GetUserData(bodyId);
}

[[nodiscard]] Vec2 bodyGetLocalPoint(BodyID bodyId, Vec2 worldPoint) NOEXCEPT
{
    return b2Body_GetLocalPoint(bodyId, worldPoint);
}

[[nodiscard]] Vec2 bodyGetWorldPoint(BodyID bodyId, Vec2 localPoint) NOEXCEPT
{
    return b2Body_GetWorldPoint(bodyId, localPoint);
}

/// Get a local vector on a body given a world vector
[[nodiscard]] Vec2 bodyGetLocalVector(BodyID bodyId, Vec2 worldVector) NOEXCEPT
{
    return b2Body_GetLocalVector(bodyId, worldVector);
}

/// Get a world vector on a body given a local vector
[[nodiscard]] Vec2 bodyGetWorldVector(BodyID bodyId, Vec2 localVector) NOEXCEPT
{
    return b2Body_GetWorldVector(bodyId, localVector);
}

[[nodiscard]] Vec2 bodyGetLinearVelocity(BodyID bodyId) NOEXCEPT
{
    return b2Body_GetLinearVelocity(bodyId);
}

[[nodiscard]] f32 bodyGetAngularVelocity(BodyID bodyId) NOEXCEPT
{
    return b2Body_GetAngularVelocity(bodyId);
}

void bodySetLinearVelocity(BodyID bodyId, Vec2 linearVelocity) NOEXCEPT
{
    b2Body_SetLinearVelocity(bodyId, linearVelocity);
}

void bodySetAngularVelocity(BodyID bodyId, f32 angularVelocity) NOEXCEPT
{
    b2Body_SetAngularVelocity(bodyId, angularVelocity);
}

void bodyEnable(BodyID body) NOEXCEPT { b2Body_Enable(body); }

void bodyDisable(BodyID body) NOEXCEPT { b2Body_Disable(body); }

[[nodiscard]] bool bodyIsEnabled(BodyID body) NOEXCEPT
{
    return b2::bodyIsEnabled(body);
}

} // namespace b2
