module;

#include "fmt_stub.h"
#include "logging_categories.h"

#include <fmt/core.h>

#include <utility>

export module logging;

#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[37m"
#define COLOR_DEFAULT "\033[39m"
#define BUFSIZE 500

namespace fmtlib = fmt::v12;

export namespace lg { // log is used by cmath, I think.
using Category = LoggingCategory;

template <typename... Args>
constexpr void debug(Category category, fmtlib::format_string<Args...> fmt,
                     Args &&...args)
{
    char buf[BUFSIZE] = {};
    fmtlib::format_to_n(buf, sizeof(buf) - 1, fmt, std::forward<Args>(args)...);
    fmtlib::println(stdout, COLOR_CYAN "[DBUG]" COLOR_DEFAULT "[{}]: {}",
                    loggingCategoryToString(category),
                    static_cast<const char *>(buf));
}

template <typename... Args>
constexpr void info(Category category, fmtlib::format_string<Args...> fmt,
                    Args &&...args)
{
    char buf[BUFSIZE] = {};
    fmtlib::format_to_n(buf, sizeof(buf) - 1, fmt, std::forward<Args>(args)...);
    stub::printNow();
    fmtlib::println(stdout, COLOR_WHITE "[INFO]" COLOR_DEFAULT "[{}]: {}",
                    loggingCategoryToString(category),
                    static_cast<const char *>(buf));
}

template <typename... Args>
constexpr void warn(Category category, fmtlib::format_string<Args...> fmt,
                    Args &&...args)
{
    char buf[BUFSIZE] = {};
    fmtlib::format_to_n(buf, sizeof(buf) - 1, fmt, std::forward<Args>(args)...);
    stub::printNow();
    fmtlib::println(stdout, COLOR_YELLOW "[WARN]" COLOR_DEFAULT "[{}]: {}",
                    loggingCategoryToString(category),
                    static_cast<const char *>(buf));
}

template <typename... Args>
constexpr void error(Category category, fmtlib::format_string<Args...> fmt,
                     Args &&...args)
{
    char buf[BUFSIZE] = {};
    fmtlib::format_to_n(buf, sizeof(buf) - 1, fmt, std::forward<Args>(args)...);
    stub::printNow();
    fmtlib::println(stdout, COLOR_RED "[EROR]" COLOR_DEFAULT "[{}]: {}",
                    loggingCategoryToString(category),
                    static_cast<const char *>(buf));
}

template <typename... Args>
constexpr void fatal(Category category, fmtlib::format_string<Args...> fmt,
                     Args &&...args)
{
    char buf[BUFSIZE] = {};
    fmtlib::format_to_n(buf, sizeof(buf) - 1, fmt, std::forward<Args>(args)...);
    stub::printNow();
    fmtlib::println(stdout, COLOR_RED "[FATAL]" COLOR_DEFAULT "[{}]: {}",
                    loggingCategoryToString(category),
                    static_cast<const char *>(buf));
}

/// Category-less printing for very quick debugging
template <typename... Args>
constexpr void print(fmtlib::format_string<Args...> fmt, Args &&...args)
{
    fmtlib::println(stdout, fmt, std::forward<Args>(args)...);
}
} // namespace lg
