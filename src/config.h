#ifndef __WERMS_CONFIG_H__
#define __WERMS_CONFIG_H__

#if defined(__linux__)
#define DLL_EXT "so"
#elif defined(__APPLE__)
#define DLL_EXT "dynlib"
#else
#define DLL_EXT "dll"
#endif

// using relative paths for now
#define WERMS_HOTRELOADABLE_DLL_PATH "build-dev/libsquinchwerms_lib." DLL_EXT
#define WERMS_SOURCE_ROOT_PATH "."
#define WERMS_HOTRELOAD_BUILD_COMMAND \
    "cmake --build build-dev --parallel --target squinchwerms_lib"

#endif
