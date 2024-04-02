cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(squinchwerms)

set(c_sources
    src/allo_impl.cpp
    src/main.cpp
    src/physics.cpp
    src/vect.cpp
    src/space.cpp
    src/shape.cpp
    src/level.cpp
    src/body.cpp
    src/natural_log/natural_log.cpp
)

set(werms_include_dirs
    src/
    )

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
set(RAYLIB_VERSION 5.0)
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
  include(FetchContent)
  FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(raylib)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
    add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
  endif()
endif()

FetchContent_Declare(
  chipmunk
  DOWNLOAD_EXTRACT_TIMESTAMP OFF
  URL https://github.com/the-argus/Chipmunk2D/archive/7acd99992e3b5923005adb5dc8ef0e8cf644a92a.tar.gz
  URL_HASH MD5=40fb3ff3f73c4ab536b41a148b2f90fd
)
FetchContent_GetProperties(chipmunk)
if (NOT chipmunk_POPULATED) # Have we downloaded raylib yet?
  set(FETCHCONTENT_QUIET NO)
  FetchContent_Populate(chipmunk)
  add_definitions(-DCP_USE_DOUBLES=0)
  set(INSTALL_DEMOS OFF CACHE BOOL "" FORCE)
  set(INSTALL_DEMOS OFF CACHE BOOL "" FORCE)
  set(BUILD_DEMOS OFF CACHE BOOL "" FORCE)
  set(BUILD_SHARED OFF CACHE BOOL "" FORCE) # BUILD_STATIC and INSTALL_STATIC are on by default
  add_subdirectory(${chipmunk_SOURCE_DIR} ${chipmunk_BINARY_DIR})
endif()

add_definitions(-DFMT_HEADER_ONLY)
FetchContent_Declare(
	fmt
    URL https://github.com/the-argus/fmt/archive/6943e150b4c9fe5b033203dabbc60668e6122a2c.tar.gz
    URL_HASH MD5=6f6caa69f1a064988fa6aadaadff63bd
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
	allo
    URL https://github.com/the-argus/allo/archive/f0de3d1cce9bbc0b2c338e8652374c9f38d22262.tar.gz
    # URL_HASH MD5=88aefe5584a49ed521bfda57dcf47cf7
)
FetchContent_MakeAvailable(allo)

FetchContent_Declare(
	ziglike
    URL https://github.com/the-argus/ziglike/archive/08cfa0318af64518ba449d4dd61a99c259466779.tar.gz
    # URL_HASH MD5=1bb3fd29973ca507ebf9f14c0704c7f0
)
FetchContent_MakeAvailable(ziglike)

# Our Project

add_executable(${PROJECT_NAME} ${c_sources})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
target_compile_definitions(${PROJECT_NAME} PRIVATE ZIGLIKE_OPTIONAL_ALLOW_POINTERS FMT_HEADER_ONLY)
#set(raylib_VERBOSE 1)
target_link_libraries(${PROJECT_NAME} raylib)
target_link_libraries(${PROJECT_NAME} chipmunk_static)
# target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
target_include_directories(${PROJECT_NAME} PRIVATE ${werms_include_dirs}) # this is a game with no deps, PRIVATE doesnt matter really
# HACK: these four shouldnt be necessary
target_include_directories(${PROJECT_NAME} PRIVATE ${chipmunk_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${allo_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${ziglike_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${fmt_SOURCE_DIR}/include)

# Web Configurations
if (${PLATFORM} STREQUAL "Web")
    # Tell Emscripten to build an example.html file.
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
endif()

# Checks if OSX and links appropriate frameworks (Only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()
