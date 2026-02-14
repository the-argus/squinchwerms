cmake_minimum_required(VERSION 3.28)

project(squinchwerms)

find_program(MOLD_PROGRAM mold)
if (MOLD_PROGRAM)
    set(CMAKE_LINKER_TYPE MOLD)
    message(STATUS "mold found: ${MOLD_PROGRAM}")
endif ()

set(hotreloaded_source_files
	# src/tesselator.cppm
	src/aliases.cppm
)
set(werms_include_dirs src/)

# the game may run `cmake --build ${BUILD_DIR} --target ${PROJECT_NAME}_lib` in
# debug mode, and re-link to it
if(CMAKE_BUILD_TYPE MATCHES "Debug")
  add_library(${PROJECT_NAME}_lib SHARED)
  add_executable(${PROJECT_NAME} src/main.cpp)
  add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_lib)
else()
  # in release mode, _lib and the executable are the same
  add_executable(${PROJECT_NAME} src/main.cpp ALIAS ${PROJECT_NAME_lib})
endif()

target_sources(${PROJECT_NAME}_lib PUBLIC FILE_SET CXX_MODULES FILES
                                          ${hotreloaded_source_files})

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(${PROJECT_NAME} PRIVATE -fmodules)
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
set_property(TARGET ${PROJECT_NAME}_lib PROPERTY CXX_STANDARD 23)

find_package(box2d REQUIRED)
find_package(SDL3 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glaze REQUIRED)
find_package(imgui REQUIRED)

target_link_libraries(${PROJECT_NAME} SDL3::SDL3 imgui) # main needs SDL and imgui, it defines entrypoints and renderer impl
target_link_libraries(${PROJECT_NAME}_lib box2d::box2d SDL3::SDL3 Vulkan::Vulkan glaze::glaze imgui)
