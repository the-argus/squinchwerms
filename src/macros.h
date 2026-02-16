#ifndef __WERMS_MACROS_H__
#define __WERMS_MACROS_H__

#include <cassert>

// werms assert
#define w_assert(cond, msg) assert(cond)

// could be disabled, like for testing and wanting to throw
#define NOEXCEPT noexcept

#endif
