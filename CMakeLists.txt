cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(squinchwerms)

set(c_sources
    src/level.c
    src/main.c
    src/physics.c
    src/rect.c
    src/allocators/global.c
    src/allocators/stack.c
    src/allocators/static_block.c
    src/memory/align.c
    src/structures/pointer_collection.c
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
  set(INSTALL_DEMOS OFF CACHE BOOL "" FORCE)
  set(BUILD_DEMOS OFF CACHE BOOL "" FORCE)
  set(BUILD_SHARED OFF CACHE BOOL "" FORCE) # BUILD_STATIC and INSTALL_STATIC are on by default
  add_subdirectory(${chipmunk_SOURCE_DIR} ${chipmunk_BINARY_DIR})
endif()

# Our Project

add_executable(${PROJECT_NAME} ${c_sources})
#set(raylib_VERBOSE 1)
target_link_libraries(${PROJECT_NAME} raylib)
target_link_libraries(${PROJECT_NAME} chipmunk_static)
target_include_directories(${PROJECT_NAME} PRIVATE ${werms_include_dirs}) # this is a game with no deps, PRIVATE doesnt matter really
target_include_directories(${PROJECT_NAME} PRIVATE ${chipmunk_SOURCE_DIR}/include)

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
