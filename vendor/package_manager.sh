#!/bin/sh

set -e

echo "engaging package management............."

# Source - https://stackoverflow.com/a/246128
# Posted by dogbane, modified by community. See post 'Timeline' for change history
# Retrieved 2026-02-23, License - CC BY-SA 4.0
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

OUTPUT_ROOT_DIR=$SCRIPT_DIR/.packages
INSTALLS_DIR=$OUTPUT_ROOT_DIR/prefixes
BUILDS_DIR=$OUTPUT_ROOT_DIR/builds

SDL_INSTALL_DIR=$INSTALLS_DIR/sdl
GLAZE_INSTALL_DIR=$INSTALLS_DIR/glaze
BOX2D_INSTALL_DIR=$INSTALLS_DIR/box2d
IMGUI_INSTALL_DIR=$INSTALLS_DIR/imgui
FMT_INSTALL_DIR=$INSTALLS_DIR/fmt

SDL_SRC_DIR=$SCRIPT_DIR/sdl
GLAZE_SRC_DIR=$SCRIPT_DIR/glaze
BOX2D_SRC_DIR=$SCRIPT_DIR/box2d
IMGUI_SRC_DIR=$SCRIPT_DIR/imgui
FMT_SRC_DIR=$SCRIPT_DIR/fmt

SDL_BUILD_DIR=$BUILDS_DIR/sdl
GLAZE_BUILD_DIR=$BUILDS_DIR/glaze
BOX2D_BUILD_DIR=$BUILDS_DIR/box2d
IMGUI_BUILD_DIR=$BUILDS_DIR/imgui
FMT_BUILD_DIR=$BUILDS_DIR/fmt

if [ ! -f $SDL_SRC_DIR/CMakeLists.txt ]; then
	echo "missing SDL, make sure to run git submodule update --init --recursive"
	exit 1
fi

mkdir -p $INSTALLS_DIR
mkdir -p $BUILDS_DIR

LINKER_TYPE="DEFAULT"

if command -v mold &> /dev/null; then
	LINKER_TYPE="MOLD"
fi

BUILD_TYPE="Debug"

# TODO: probably be aggressive here and enable features so that errors happen if
# you do not have all dependencies installed. requires checking for current
# platform. or grep for things in CMakeCache.txt after configure step

# configure SDL
cmake -S $SDL_SRC_DIR -B $SDL_BUILD_DIR \
	-DCMAKE_LINKER_TYPE=$LINKER_TYPE \
	-DCMAKE_BUILD_TYPE=$BUILD_TYPE \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	-DSDL_ASAN=OFF \
	-DSDL_ASSERTIONS=paranoid \
	-DSDL_TESTS=OFF \
	-DSDL_EXAMPLES=OFF \
	-DSDL_OPENGL=OFF \
	-DSDL_OPENGLES=OFF \
	-DSDL_OSS=OFF \
	-DSDL_ALSA=OFF \
	-DSDL_ALSA_SHARED=OFF \
	-DSDL_JACK=OFF \
	-DSDL_JACK_SHARED=OFF \
	-DSDL_X11_XSCRNSAVER=OFF \
	-DSDL_X11_XSHAPE=OFF \
	-DSDL_X11_XSYNC=OFF \
	-DSDL_X11_XDBE=OFF \
	-DSDL_X11_XTEST=OFF
# build SDL
cmake --build $SDL_BUILD_DIR --parallel
# install SDL
cmake --install $SDL_BUILD_DIR --prefix $SDL_INSTALL_DIR

# configure glaze
cmake -S $GLAZE_SRC_DIR -B $GLAZE_BUILD_DIR \
	-DCMAKE_LINKER_TYPE=$LINKER_TYPE \
	-DCMAKE_BUILD_TYPE=$BUILD_TYPE \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	-Dglaze_DEVELOPER_MODE=OFF \
	-Dglaze_DISABLE_ALWAYS_INLINE=ON \
	-Dglaze_BUILD_EXAMPLES=OFF \
	-Dglaze_ENABLE_SSL=OFF
# build glaze
cmake --build $GLAZE_BUILD_DIR --parallel
# install glaze
cmake --install $GLAZE_BUILD_DIR --prefix $GLAZE_INSTALL_DIR

# configure box2d
cmake -S $BOX2D_SRC_DIR -B $BOX2D_BUILD_DIR \
	-DCMAKE_BUILD_TYPE=$BUILD_TYPE \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	-DCMAKE_LINKER_TYPE=$LINKER_TYPE \
	-DBOX2D_SAMPLES=OFF \
	-DBOX2D_BENCHMARKS=OFF \
	-DBOX2D_DOCS=OFF \
	-DBOX2D_PROFILE=OFF \
	-DBOX2D_VALIDATE=ON \
	-DBOX2D_UNIT_TESTS=OFF
# build box2d
cmake --build $BOX2D_BUILD_DIR --parallel
# install box2d
cmake --install $BOX2D_BUILD_DIR --prefix $BOX2D_INSTALL_DIR

#configure imgui
cmake -S $IMGUI_SRC_DIR -B $IMGUI_BUILD_DIR \
	-DCMAKE_BUILD_TYPE=$BUILD_TYPE \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	-DCMAKE_LINKER_TYPE=$LINKER_TYPE \
	-DCMAKE_PREFIX_PATH=$SDL_INSTALL_DIR \
	-DIMGUI_BUILD_VULKAN_BINDING=ON \
	-DIMGUI_BUILD_GLFW_BINDING=OFF \
	-DIMGUI_BUILD_SDL3_BINDING=ON \
	-DIMGUI_BUILD_SDL3_RENDERER_BINDING=ON
# build imgui
cmake --build $IMGUI_BUILD_DIR --parallel
# install imgui
cmake --install $IMGUI_BUILD_DIR --prefix $IMGUI_INSTALL_DIR

# configure fmt
cmake -S $FMT_SRC_DIR -B $FMT_BUILD_DIR \
	-DCMAKE_BUILD_TYPE=$BUILD_TYPE \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	-DCMAKE_LINKER_TYPE=$LINKER_TYPE \
	-DFMT_DOC=OFF \
	-DFMT_TEST=OFF \
	-DFMT_FUZZ=OFF \
	-DFMT_CUDA_TEST=OFF \
	-DFMT_UNICODE=ON

# build fmt
cmake --build $FMT_BUILD_DIR --parallel
# install fmt
cmake --install $FMT_BUILD_DIR --prefix $FMT_INSTALL_DIR

OUTSTRINGS="$OUTPUT_ROOT_DIR/cmake_prefix_path"

rm -f $OUTSTRINGS
echo "$SDL_INSTALL_DIR" >> $OUTSTRINGS
echo "$GLAZE_INSTALL_DIR" >> $OUTSTRINGS
echo "$BOX2D_INSTALL_DIR" >> $OUTSTRINGS
echo "$IMGUI_INSTALL_DIR" >> $OUTSTRINGS
echo "$FMT_INSTALL_DIR" >> $OUTSTRINGS
