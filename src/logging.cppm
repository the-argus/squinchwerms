module;

#include "logging_categories.h"
#include <print>
#include <utility>

export module logging;

#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[37m"
#define COLOR_DEFAULT "\033[39m"
#define BUFSIZE 500

export namespace lg { // log is used by cmath, I think.
using Category = LoggingCategory;

template <typename... Args>
void debug(Category category, std::format_string<Args...> fmt, Args &&...args)
{
    char buf[BUFSIZE] = {};
    std::format_to_n(buf, sizeof(buf) - 1, fmt, std::forward<Args>(args)...);
    std::println(stdout, COLOR_CYAN "[DBUG]" COLOR_DEFAULT "[{}]: {}",
                 loggingCategoryToString(category),
                 static_cast<const char *>(buf));
}

template <typename... Args>
void info(Category category, std::format_string<Args...> fmt, Args &&...args)
{
    char buf[BUFSIZE] = {};
    std::format_to_n(buf, sizeof(buf) - 1, fmt, std::forward<Args>(args)...);
    std::println(stdout, COLOR_WHITE "[INFO]" COLOR_DEFAULT "[{}]: {}",
                 loggingCategoryToString(category),
                 static_cast<const char *>(buf));
}

template <typename... Args>
constexpr void warn(Category category, std::format_string<Args...> fmt,
                    Args &&...args)
{
    char buf[BUFSIZE] = {};
    std::format_to_n(buf, sizeof(buf) - 1, fmt, std::forward<Args>(args)...);
    std::println(stdout, COLOR_YELLOW "[WARN]" COLOR_DEFAULT "[{}]: {}",
                 loggingCategoryToString(category),
                 static_cast<const char *>(buf));
}

template <typename... Args>
constexpr void error(Category category, std::format_string<Args...> fmt,
                     Args &&...args)
{
    char buf[BUFSIZE] = {};
    std::format_to_n(buf, sizeof(buf) - 1, fmt, std::forward<Args>(args)...);
    std::println(stdout, COLOR_RED "[EROR]" COLOR_DEFAULT "[{}]: {}",
                 loggingCategoryToString(category),
                 static_cast<const char *>(buf));
}

template <typename... Args>
constexpr void fatal(Category category, std::format_string<Args...> fmt,
                     Args &&...args)
{
    char buf[BUFSIZE] = {};
    std::format_to_n(buf, sizeof(buf) - 1, fmt, std::forward<Args>(args)...);
    std::println(stdout, COLOR_RED "[FATAL]" COLOR_DEFAULT "[{}]: {}",
                 loggingCategoryToString(category),
                 static_cast<const char *>(buf));
}

/// Category-less printing for very quick debugging
template <typename... Args>
constexpr void print(std::format_string<Args...> fmt, Args &&...args)
{
    std::println(stdout, fmt, std::forward<Args>(args)...);
}
} // namespace lg
