cmake_minimum_required(VERSION 3.28)

project(squinchwerms)

set(PACKAGESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/.packages/cmake_prefix_path")
if (EXISTS ${PACKAGESOURCE})
	message(STATUS "File ${PACKAGESOURCE} exists")
	file(STRINGS ${PACKAGESOURCE} ADDITIONAL_PREFIX_PATH)
	foreach(PREFIX_PATH_ENTRY ${ADDITIONAL_PREFIX_PATH})
		message(STATUS "Adding ${PREFIX_PATH_ENTRY} to CMAKE_PREFIX_PATH")
		list(APPEND CMAKE_PREFIX_PATH "${PREFIX_PATH_ENTRY}")
	endforeach()
endif()

find_program(MOLD_PROGRAM mold)
if(MOLD_PROGRAM)
  set(CMAKE_LINKER_TYPE MOLD)
  message(STATUS "mold found: ${MOLD_PROGRAM}")
endif()

set(hotreloaded_source_files # src/tesselator.cppm
    src/aliases.cppm
    src/lib/uninitialized_storage.cppm
    src/lib/std_arraylike_container.cppm
    src/lib/is_instance.cppm
    src/allocators/allocator.cppm
	# src/allocators/arena.cppm
    src/lib/slice.cppm
    src/lib/opt.cppm
    src/lib/res.cppm
    src/library_entry.cppm
    src/main_menu.cppm
    src/logging.cppm)
set(werms_include_dirs src/)

# the game may run `cmake --build ${BUILD_DIR} --target ${PROJECT_NAME}_lib` in
# debug mode, and re-link to it
if(CMAKE_BUILD_TYPE MATCHES "Debug")
  add_library(${PROJECT_NAME}_lib SHARED)
  add_executable(${PROJECT_NAME} src/main.cpp)
  add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_lib)
else()
  # in release mode, _lib and the executable are the same
  add_executable(${PROJECT_NAME} src/main.cpp ALIAS ${PROJECT_NAME}_lib)
endif()

target_sources(${PROJECT_NAME}_lib PUBLIC FILE_SET CXX_MODULES FILES
                                          ${hotreloaded_source_files})
# get around fmt headers having a static symbol in them
target_sources(${PROJECT_NAME}_lib PUBLIC src/fmt_stub.cpp)

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES CXX_STANDARD_REQUIRED ON
             CXX_STANDARD 20
             CXX_EXTENSIONS OFF
             CXX_SCAN_FOR_MODULES ON
             CXX_MODULE_STD OFF)
set_target_properties(
  ${PROJECT_NAME}_lib
  PROPERTIES CXX_STANDARD_REQUIRED ON
             CXX_STANDARD 20
             CXX_EXTENSIONS OFF
             CXX_SCAN_FOR_MODULES ON
             CXX_MODULE_STD OFF)

target_include_directories(${PROJECT_NAME}_lib PRIVATE ${werms_include_dirs})

find_package(box2d REQUIRED)
find_package(SDL3 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glaze REQUIRED)
find_package(imgui REQUIRED)
find_package(fmt REQUIRED)

# main needs sdl and imgui because those have some static/global initialization
# that needs to stick around
target_link_libraries(${PROJECT_NAME} SDL3::SDL3 imgui::imgui fmt::fmt)
target_link_libraries(${PROJECT_NAME}_lib box2d::box2d SDL3::SDL3
                      Vulkan::Vulkan glaze::glaze imgui::imgui fmt::fmt)
