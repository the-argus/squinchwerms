#pragma once
#include <fmt/format.h>

#include <okay/short_arithmetic_types.h>

namespace ln {
// not doing INFO = ::LOG_INFO here because I dont want to have to include
// the raylib header in this wrapper
enum class LogLevel : u8
{
    ALL = 0,
    TRACE = 1,
    DEBUG = 2,
    INFO = 3,
    WARNING = 4,
    ERROR = 5,
    FATAL = 6,
    NONE = 7,
};

///
/// Register the logger through raylib.
///
void init() noexcept;

///
/// Log a message with a given urgency.
///
void log(LogLevel level, fmt::string_view message) noexcept;

///
/// Log a message with info level urgency.
///
constexpr void log(fmt::string_view message) noexcept
{
    log(LogLevel::INFO, message);
}

namespace detail {
static constexpr u64 bufsize = 512;
// NOLINTNEXTLINE
extern thread_local char threadbuf[bufsize];
}; // namespace detail

///
/// Log a message with a given urgency while also performing a string format.
/// Prints into a stack buffer. Template argument buffersize determines the max
/// bytes of that buffer. If it overwrites the buffer, the message will be
/// truncated.
///
template <typename... T>
constexpr void logFormatted(LogLevel level, fmt::format_string<T...> fmt_string,
                         T &&...args) noexcept
{
    using namespace detail;
    auto result = fmt::format_to_n(threadbuf, bufsize, fmt_string,
                                   std::forward<decltype(args)>(args)...);
    // make sure last char is nul byte so its convertible to cstring
    threadbuf[bufsize - 1] = 0;
    u64 bytes_printed = result.out - threadbuf;
    log(level,
        std::string_view(threadbuf,
                         bytes_printed >= bufsize ? bufsize : bytes_printed));
}

///
/// Set level of urgency below which log messages will not be printed.
///
void setMinimumLevel(LogLevel level) noexcept;

// clang-format off
// TODO: add macro logic for making some of these be no-ops if you define a
// compile-time minimum log level
#define LN_DEBUG(fmtstring) ln::log(ln::LogLevel::DEBUG, fmtstring)
#define LN_DEBUG_FMT(fmtstring, ...) ln::logFormatted(ln::LogLevel::DEBUG, fmtstring, __VA_ARGS__)
#define LN_WARN(fmtstring) ln::log(ln::LogLevel::WARNING, fmtstring)
#define LN_WARN_FMT(fmtstring, ...) ln::logFormatted(ln::LogLevel::WARNING, fmtstring, __VA_ARGS__)
#define LN_ERROR(fmtstring) ln::log(ln::LogLevel::ERROR, fmtstring)
#define LN_ERROR_FMT(fmtstring, ...) ln::logFormatted(ln::LogLevel::ERROR, fmtstring, __VA_ARGS__)
#define LN_FATAL(fmtstring) ln::log(ln::LogLevel::FATAL, fmtstring)
#define LN_FATAL_FMT(fmtstring, ...) ln::logFormatted(ln::LogLevel::FATAL, fmtstring, __VA_ARGS__)
#define LN_INFO(fmtstring) ln::log(ln::LogLevel::INFO, fmtstring)
#define LN_INFO_FMT(fmtstring, ...) ln::logFormatted(ln::LogLevel::INFO, fmtstring, __VA_ARGS__)
// clang-format on

} // namespace ln
