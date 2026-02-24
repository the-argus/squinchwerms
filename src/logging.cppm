module;

#include "logging_categories.h"

#include <chrono>
#include <utility>

#include <fmt/chrono.h>
#include <fmt/core.h>

export module logging;

#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[37m"
#define COLOR_DEFAULT "\033[39m"
#define BUFSIZE 500
#define TIME_FORMAT "[{:%H:%M:%S}]"

constexpr auto now() noexcept
{
    return std::chrono::time_point_cast<std::chrono::duration<int64_t>>(
        std::chrono::system_clock::now());
}

export namespace lg { // log is used by cmath, I think.
using Category = LoggingCategory;

template <typename... Args>
void debug(Category category, fmt::format_string<Args...> fmt, Args &&...args)
{
    char buf[BUFSIZE] = {};
    fmt::format_to_n(buf, sizeof(buf) - 1, fmt, std::forward<Args>(args)...);
    fmt::println(
        stdout, TIME_FORMAT COLOR_CYAN "[DBUG]" COLOR_DEFAULT "[{}]: {}", now(),
        loggingCategoryToString(category), static_cast<const char *>(buf));
}

template <typename... Args>
void info(Category category, fmt::format_string<Args...> fmt, Args &&...args)
{
    char buf[BUFSIZE] = {};
    fmt::format_to_n(buf, sizeof(buf) - 1, fmt, std::forward<Args>(args)...);
    fmt::println(stdout,
                 TIME_FORMAT COLOR_WHITE "[INFO]" COLOR_DEFAULT "[{}]: {}",
                 now(), loggingCategoryToString(category),
                 static_cast<const char *>(buf));
}

template <typename... Args>
constexpr void warn(Category category, fmt::format_string<Args...> fmt,
                    Args &&...args)
{
    char buf[BUFSIZE] = {};
    fmt::format_to_n(buf, sizeof(buf) - 1, fmt, std::forward<Args>(args)...);
    fmt::println(stdout,
                 TIME_FORMAT COLOR_YELLOW "[WARN]" COLOR_DEFAULT "[{}]: {}",
                 now(), loggingCategoryToString(category),
                 static_cast<const char *>(buf));
}

template <typename... Args>
constexpr void error(Category category, fmt::format_string<Args...> fmt,
                     Args &&...args)
{
    char buf[BUFSIZE] = {};
    fmt::format_to_n(buf, sizeof(buf) - 1, fmt, std::forward<Args>(args)...);
    fmt::println(
        stdout, TIME_FORMAT COLOR_RED "[EROR]" COLOR_DEFAULT "[{}]: {}", now(),
        loggingCategoryToString(category), static_cast<const char *>(buf));
}

template <typename... Args>
constexpr void fatal(Category category, fmt::format_string<Args...> fmt,
                     Args &&...args)
{
    char buf[BUFSIZE] = {};
    fmt::format_to_n(buf, sizeof(buf) - 1, fmt, std::forward<Args>(args)...);
    fmt::println(
        stdout, TIME_FORMAT COLOR_RED "[FATAL]" COLOR_DEFAULT "[{}]: {}", now(),
        loggingCategoryToString(category), static_cast<const char *>(buf));
}

/// Category-less printing for very quick debugging
template <typename... Args>
constexpr void print(fmt::format_string<Args...> fmt, Args &&...args)
{
    fmt::println(stdout, fmt, std::forward<Args>(args)...);
}
} // namespace lg
