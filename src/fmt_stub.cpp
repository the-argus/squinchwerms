// we include this here because there is an inline function with an internal
// static variable in fmt, which becomes a symbol that the hotreloadable game
// lib depends on, but no one exports it... idk. by doing this in a traditional
// c++ file, we build it somewhere.
#include <fmt/chrono.h>

namespace stub {

// NOTE: for reasons I can only imagine, this does not compile if it is inside
// one of the exported and templated log functions in logging.cppm below due to
// not finding operator- for std::chrono::duration.
void printNow() noexcept
{
    fmt::println(stdout, "[{:%H:%M:%S}]", std::chrono::system_clock::now());
}
} // namespace stub
