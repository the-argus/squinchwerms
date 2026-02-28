#ifndef __WERMS_MACROS_H__
#define __WERMS_MACROS_H__

#include <cassert>
#include <cstdlib> // abort

// werms assert
#define w_assert(cond, msg) assert(cond)

// werms abort
#define w_abort(msg) ::abort()

// could be disabled, like for testing and wanting to throw
#define NOEXCEPT noexcept

#endif
