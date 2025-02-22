#include "natural_log/natural_log.h"
#include <array>
#include <atomic>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <mutex>
#include <raylib.h>

namespace ln::detail {
// NOLINTNEXTLINE
thread_local char threadbuf[bufsize];
}; // namespace ln::detail

static void internal(int msgType, const char *text, va_list args) noexcept;
static std::atomic<ln::LogLevel> minLevel = ln::LogLevel::ALL;

// make sure raylib matches up with our types so we can convert safely
// clang-format off
static_assert(std::underlying_type_t<ln::LogLevel>(ln::LogLevel::ALL) == static_cast<int>(::LOG_ALL));
static_assert(std::underlying_type_t<ln::LogLevel>(ln::LogLevel::TRACE) == static_cast<int>(::LOG_TRACE));
static_assert(std::underlying_type_t<ln::LogLevel>(ln::LogLevel::DEBUG) == static_cast<int>(::LOG_DEBUG));
static_assert(std::underlying_type_t<ln::LogLevel>(ln::LogLevel::INFO) == static_cast<int>(::LOG_INFO));
static_assert(std::underlying_type_t<ln::LogLevel>(ln::LogLevel::WARNING) == static_cast<int>(::LOG_WARNING));
static_assert(std::underlying_type_t<ln::LogLevel>(ln::LogLevel::ERROR) == static_cast<int>(::LOG_ERROR));
static_assert(std::underlying_type_t<ln::LogLevel>(ln::LogLevel::FATAL) == static_cast<int>(::LOG_FATAL));
static_assert(std::underlying_type_t<ln::LogLevel>(ln::LogLevel::NONE) == static_cast<int>(::LOG_NONE));
// clang-format on

namespace ln {

// tell raylib to use our internal colored logger
void init() noexcept { SetTraceLogCallback(internal); }

void set_minimum_level(LogLevel level) noexcept
{
    SetTraceLogLevel(std::underlying_type_t<LogLevel>(level));
    minLevel = level;
}

void log(LogLevel level, fmt::string_view message) noexcept
{
    static std::mutex logmutex;
    std::lock_guard lock(logmutex);
    if (level <= minLevel)
        return;

    {
        std::time_t currenttime = std::time(nullptr);
        fmt::print("[{:%H:%M:%S}] ", fmt::localtime(currenttime));
    }

    switch (level) {
    case LogLevel::INFO:
        fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::white),
                   "[INFO] : ");
        break;
    case LogLevel::ERROR:
        fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::red),
                   "[ERROR] : ");
        break;
    case LogLevel::WARNING:
        fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::yellow),
                   "[WARN] : ");
        break;
    case LogLevel::DEBUG:
        fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::green),
                   "[DEBUG] : ");
        break;
    case LogLevel::FATAL:
        fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::red),
                   "[FATAL] : ");
        break;
    case LogLevel::TRACE:
        fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::cyan),
                   "[TRACE] : ");
        break;
    case LogLevel::NONE:
        fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::yellow),
                   "[UNDEFINED NONELEVEL] : ");
        break;
    default:
        fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::yellow),
                   "[WARN] : (Unimplemented log level used!) ");
        break;
    }

    // use cout because its a string view and printf doesnt know how to do that
    fmt::println("{}", message);
}
} // namespace ln

static void internal(int msgType, const char *text, va_list args) noexcept
{
    std::array<char, 512> buffer;
    int bytesPrinted = vsnprintf(buffer.data(), buffer.size(), text, args);
    if (bytesPrinted < 0) {
        ln::log(ln::LogLevel::WARNING,
                "encoding error occured when trying to print string");
        return;
    }

    if (bytesPrinted > buffer.size())
        log(ln::LogLevel::WARNING,
            "Failed to completely print the following message:");

    ln::log(static_cast<ln::LogLevel>(msgType), buffer.data());
}
