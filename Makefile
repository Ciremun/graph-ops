#
# Makefile to use with emscripten
# See https://emscripten.org/docs/getting_started/downloads.html
# for installation instructions.
#
# This Makefile assumes you have loaded emscripten's environment.
# (On Windows, you may need to execute emsdk_env.bat or encmdprompt.bat ahead)
#
# Running `make` will produce three files:
#  - web/index.html
#  - web/index.js
#  - web/index.wasm
#
# All three are needed to run the demo.

CXX = em++
WEB_DIR = docs
EXE = $(WEB_DIR)/index.html
SOURCES = src/backends/impl_emscripten.cpp src/common/io.cpp src/common/sphere.cpp src/common/shader.cpp
SOURCES += imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/backends/imgui_impl_sdl.cpp imgui/backends/imgui_impl_opengl3.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
UNAME_S := $(shell uname -s)
CPPFLAGS =-DIMGUI_USE_STB_SPRINTF -DIMGUI_DEFINE_MATH_OPERATORS
LDFLAGS =
EMS =

##---------------------------------------------------------------------
## EMSCRIPTEN OPTIONS
##---------------------------------------------------------------------

# ("EMS" options gets added to both CPPFLAGS and LDFLAGS, whereas some options are for linker only)
EMS += -s USE_SDL=2 -s MAX_WEBGL_VERSION=2 -s MIN_WEBGL_VERSION=2
EMS += -s DISABLE_EXCEPTION_CATCHING=1
LDFLAGS += -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=0 -s ASSERTIONS=1

# Uncomment next line to fix possible rendering bugs with Emscripten version older then 1.39.0 (https://github.com/ocornut/imgui/issues/2877)
#EMS += -s BINARYEN_TRAP_MODE=clamp
#EMS += -s SAFE_HEAP=1    ## Adds overhead

# Emscripten allows preloading a file or folder to be accessible at runtime.
# The Makefile for this example project suggests embedding the misc/fonts/ folder into our application, it will then be accessible as "/fonts"
# See documentation for more details: https://emscripten.org/docs/porting/files/packaging_files.html
# (Default value is 0. Set to 1 to enable file-system and include the misc/fonts/ folder as part of the build.)
USE_FILE_SYSTEM ?= 1
ifeq ($(USE_FILE_SYSTEM), 0)
LDFLAGS += -s NO_FILESYSTEM=1
CPPFLAGS += -DIMGUI_DISABLE_FILE_FUNCTIONS
endif
ifeq ($(USE_FILE_SYSTEM), 1)
LDFLAGS += --no-heap-copy --preload-file src/shaders/frag.glsl --preload-file src/shaders/vert.glsl --preload-file imgui/fonts/Roboto-Medium.ttf
endif

##---------------------------------------------------------------------
## FINAL BUILD FLAGS
##---------------------------------------------------------------------

CPPFLAGS += -Isrc/include -Iimgui -Iimgui/backends
#CPPFLAGS += -g
CPPFLAGS += -Wall -Wformat -Os $(EMS)
LDFLAGS += --shell-file src/shell_minimal.html $(EMS)

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

%.o:src/common/%.cpp
	$(CXX) $(CPPFLAGS) -c -o $@ $<

%.o:src/backends/%.cpp
	$(CXX) $(CPPFLAGS) -c -o $@ $<

%.o:imgui/%.cpp
	$(CXX) $(CPPFLAGS) -c -o $@ $<

%.o:imgui/backends/%.cpp
	$(CXX) $(CPPFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(EXE)

$(WEB_DIR):
	mkdir -p $@

serve: all
	python3 -m http.server -d $(WEB_DIR)

$(EXE): $(OBJS) $(WEB_DIR)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -rf $(OBJS) $(WEB_DIR)
