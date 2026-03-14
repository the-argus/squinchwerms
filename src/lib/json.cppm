module;

#include <array>
#include <fmt/base.h>
#include <glaze/core/istream_buffer.hpp>
#include <glaze/core/ostream_buffer.hpp>
#include <glaze/glaze.hpp>
#include <glaze/json.hpp>
#include <glaze/json/write.hpp>
#include <span>

#include "macros.h"

export module json;

import slice;
import res;
import logging;
import aliases;
import reflection;

export enum class IOStatus {
    Success,
    Error, // dont care
    EndOfBufferReached,
};

export template <typename Struct>
[[nodiscard]] constexpr IOStatus writeJson(const Struct &value,
                                           const char *fileName)
{
    std::ofstream file(fileName);
    glz::basic_ostream_buffer<std::ofstream> buffer(file);
    auto ec = glz::write_json(value, buffer);
    if (ec || !file.good())
        return IOStatus::Error;

    return IOStatus::Success;
}

/// write json to buffer, returning IOStatus::EndOfBufferReached if unable to
/// write all the json. if successful, returns slice of bytes written to
export template <typename Struct>
[[nodiscard]] constexpr Res<Slice<char>, IOStatus>
writeJsonToBuffer(const Struct &value, Slice<char> chars)
{
    if (chars.isEmpty()) [[unlikely]]
        return IOStatus::EndOfBufferReached;

    auto span = std::span{
        chars.uncheckedAddressOfFirstItem(),
        chars.uncheckedAddressOfFirstItem() + chars.size(),
    };
    glz::error_ctx ec = glz::write_json(value, span);

    if (ec) {
        lg::error(lg::Category::Serialization,
                  "Failed to write type {} to buffer, failed after {} bytes, "
                  "error code {} and message: {}",
                  typeName<Struct>(), ec.count, static_cast<u32>(ec),
                  ec.custom_error_message);
        return IOStatus::Error;
    }

    if (ec.count == 0) {
        return makeNullSlice<char>();
    }

    return unsafe::rawSlice(*span.data(), ec.count);
}

namespace json {
struct TestStruct
{
    i32 i;
    f32 f;
};

struct TestWithPointer
{
    i32 i;
    TestStruct *testPointer;
};

void testJsonWriteToBuffer()
{
    TestStruct test{1, 2.0f};

    std::array<char, 500> buf;
    Res bytes = writeJsonToBuffer(test, buf);

    w_assert(isSuccess(bytes), "");
}

void testJsonWriteToBufferWithPointer()
{
    TestStruct a{3, 2};
    TestWithPointer withPointer{30, &a};

    std::array<char, 500> buf = {};
    Res bytes = writeJsonToBuffer(withPointer, buf);
    w_assert(isSuccess(bytes), "");

    withPointer.testPointer = nullptr;
    buf = {};
    Res bytes2 = writeJsonToBuffer(withPointer, buf);
    w_assert(isSuccess(bytes2), "");

    Res bytes3 = writeJsonToBuffer(a, buf);
    w_assert(isSuccess(bytes3), "");
}

void testJsonWriteToFile()
{
    TestStruct test{1, 2.0f};

    std::array<char, 500> filename;
    fmt::format_to_n(filename.begin(), filename.size(), "build-dev/{}.json",
                     __func__);

    const auto status = writeJson(test, filename.data());
    w_assert(status == IOStatus::Success, "");
}
} // namespace json

export namespace tests {
void json()
{
    json::testJsonWriteToBuffer();
    json::testJsonWriteToFile();
    json::testJsonWriteToBufferWithPointer();
}
} // namespace tests
