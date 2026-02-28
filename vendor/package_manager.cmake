cmake_minimum_required(VERSION 3.14)

get_filename_component(SCRIPT_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)

set(OUTPUT_ROOT_DIR "${SCRIPT_DIR}/.packages")
set(INSTALLS_DIR "${OUTPUT_ROOT_DIR}/prefixes")
set(BUILDS_DIR "${OUTPUT_ROOT_DIR}/builds")

set(SDL_INSTALL_DIR "${INSTALLS_DIR}/sdl")
set(GLAZE_INSTALL_DIR "${INSTALLS_DIR}/glaze")
set(BOX2D_INSTALL_DIR "${INSTALLS_DIR}/box2d")
set(IMGUI_INSTALL_DIR "${INSTALLS_DIR}/imgui")
set(FMT_INSTALL_DIR "${INSTALLS_DIR}/fmt")

set(SDL_SRC_DIR "${SCRIPT_DIR}/sdl")
set(GLAZE_SRC_DIR "${SCRIPT_DIR}/glaze")
set(BOX2D_SRC_DIR "${SCRIPT_DIR}/box2d")
set(IMGUI_SRC_DIR "${SCRIPT_DIR}/imgui")
set(FMT_SRC_DIR "${SCRIPT_DIR}/fmt")

set(SDL_BUILD_DIR "${BUILDS_DIR}/sdl")
set(GLAZE_BUILD_DIR "${BUILDS_DIR}/glaze")
set(BOX2D_BUILD_DIR "${BUILDS_DIR}/box2d")
set(IMGUI_BUILD_DIR "${BUILDS_DIR}/imgui")
set(FMT_BUILD_DIR "${BUILDS_DIR}/fmt")

if(NOT EXISTS "${SDL_SRC_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "missing SDL, make sure to run git submodule update --init --recursive")
endif()

file(MAKE_DIRECTORY ${INSTALLS_DIR})
file(MAKE_DIRECTORY ${BUILDS_DIR})

find_program(MOLD_EXECUTABLE mold)
if(MOLD_EXECUTABLE)
    set(LINKER_TYPE "MOLD")
else()
    set(LINKER_TYPE "DEFAULT")
endif()

set(BUILD_TYPE "Debug")

set(SDL_COMMON_FLAGS
	-DSDL_TESTS=OFF
	-DSDL_EXAMPLES=OFF
	-DSDL_VULKAN=ON
	-DSDL_RENDER_VULKAN=ON
	-DSDL_OPENGL=OFF
	-DSDL_OPENGLES=OFF
	-DSDL_OSS=OFF
	-DSDL_ALSA=OFF
	-DSDL_ALSA_SHARED=OFF
	-DSDL_JACK=OFF
	-DSDL_JACK_SHARED=OFF
	-DSDL_X11_XSCRNSAVER=OFF
	-DSDL_X11_XSHAPE=OFF
	-DSDL_X11_XSYNC=OFF
	-DSDL_X11_XDBE=OFF
	-DSDL_X11_XTEST=OFF)

if(WIN32)
	set(SDL_FLAGS ${SDL_COMMON_FLAGS}
    )
elseif(APPLE)
    set(SDL_FLAGS
		${SDL_COMMON_FLAGS}
    )
else()
    set(SDL_FLAGS
		${SDL_COMMON_FLAGS}
        -DSDL_X11=ON
		-DSDL_X11_SHARED=OFF
        -DSDL_WAYLAND=ON
    )
endif()

# Configure SDL
message(STATUS "Configuring SDL...")
execute_process(
    COMMAND cmake -S ${SDL_SRC_DIR} -B ${SDL_BUILD_DIR}
    -DCMAKE_LINKER_TYPE=${LINKER_TYPE}
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    ${SDL_FLAGS}
)
execute_process(COMMAND cmake --build ${SDL_BUILD_DIR} --parallel)
execute_process(COMMAND cmake --install ${SDL_BUILD_DIR} --prefix ${SDL_INSTALL_DIR})

# Configure, build, and install Glaze
message(STATUS "Configuring Glaze...")
execute_process(
    COMMAND cmake -S ${GLAZE_SRC_DIR} -B ${GLAZE_BUILD_DIR}
    -DCMAKE_LINKER_TYPE=${LINKER_TYPE}
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    -Dglaze_DEVELOPER_MODE=OFF
    -Dglaze_DISABLE_ALWAYS_INLINE=ON
    -Dglaze_BUILD_EXAMPLES=OFF
    -Dglaze_ENABLE_SSL=OFF
)
execute_process(COMMAND cmake --build ${GLAZE_BUILD_DIR} --parallel)
execute_process(COMMAND cmake --install ${GLAZE_BUILD_DIR} --prefix ${GLAZE_INSTALL_DIR})

# Configure, build, and install Box2D
message(STATUS "Configuring Box2D...")
execute_process(
    COMMAND cmake -S ${BOX2D_SRC_DIR} -B ${BOX2D_BUILD_DIR}
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    -DCMAKE_LINKER_TYPE=${LINKER_TYPE}
    -DBOX2D_SAMPLES=OFF
    -DBOX2D_BENCHMARKS=OFF
    -DBOX2D_DOCS=OFF
    -DBOX2D_PROFILE=OFF
    -DBOX2D_VALIDATE=ON
    -DBOX2D_UNIT_TESTS=OFF
)
execute_process(COMMAND cmake --build ${BOX2D_BUILD_DIR} --parallel)
execute_process(COMMAND cmake --install ${BOX2D_BUILD_DIR} --prefix ${BOX2D_INSTALL_DIR})

# Configure, build, and install ImGui
message(STATUS "Configuring ImGui...")
execute_process(
    COMMAND cmake -S ${IMGUI_SRC_DIR} -B ${IMGUI_BUILD_DIR}
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    -DCMAKE_LINKER_TYPE=${LINKER_TYPE}
    -DCMAKE_PREFIX_PATH=${SDL_INSTALL_DIR}
    -DIMGUI_BUILD_VULKAN_BINDING=ON
    -DIMGUI_BUILD_GLFW_BINDING=OFF
    -DIMGUI_BUILD_SDL3_BINDING=ON
    -DIMGUI_BUILD_SDL3_RENDERER_BINDING=ON
)
execute_process(COMMAND cmake --build ${IMGUI_BUILD_DIR} --parallel)
execute_process(COMMAND cmake --install ${IMGUI_BUILD_DIR} --prefix ${IMGUI_INSTALL_DIR})

# Configure, build, and install fmt
message(STATUS "Configuring fmt...")
execute_process(
    COMMAND cmake -S ${FMT_SRC_DIR} -B ${FMT_BUILD_DIR}
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    -DCMAKE_LINKER_TYPE=${LINKER_TYPE}
    -DFMT_DOC=OFF
    -DFMT_TEST=OFF
    -DFMT_FUZZ=OFF
    -DFMT_CUDA_TEST=OFF
    -DFMT_UNICODE=ON
)
execute_process(COMMAND cmake --build ${FMT_BUILD_DIR} --parallel)
execute_process(COMMAND cmake --install ${FMT_BUILD_DIR} --prefix ${FMT_INSTALL_DIR})

# Write cmake prefix path to output file
set(OUTSTRINGS "${OUTPUT_ROOT_DIR}/cmake_prefix_path")
file(REMOVE ${OUTSTRINGS})
file(APPEND ${OUTSTRINGS} "${SDL_INSTALL_DIR}\n")
file(APPEND ${OUTSTRINGS} "${GLAZE_INSTALL_DIR}\n")
file(APPEND ${OUTSTRINGS} "${BOX2D_INSTALL_DIR}\n")
file(APPEND ${OUTSTRINGS} "${IMGUI_INSTALL_DIR}\n")
file(APPEND ${OUTSTRINGS} "${FMT_INSTALL_DIR}\n")
