SOURCES = source/imgui/imgui.cpp source/imgui/imgui_draw.cpp source/imgui/imgui_tables.cpp source/imgui/imgui_widgets.cpp
SOURCES += source/imgui/backends/imgui_impl_glfw.cpp source/imgui/backends/imgui_impl_opengl3.cpp
SOURCES += source/common/model.cpp source/common/io.cpp source/common/shader.cpp source/common/update.cpp
SOURCES += source/common/aabb.cpp source/common/ray.cpp source/common/line.cpp
SOURCES += source/backends/impl_glfw.cpp

CXXFLAGS = -Isource/imgui -Isource/imgui/backends -Iinclude
CXXFLAGS += -DIMGUI_USE_STB_SPRINTF
CXXFLAGS += -Wall -Wextra -pedantic -std=c++17 -ggdb -O0

EXE = graph-ops

OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

ifeq ($(OS),Windows_NT)
	LIBS = -lglew32 -lglfw3 -lopengl32
else
	LIBS = -lGLEW -lglfw -lGL -ldl
endif

%.o:source/common/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:source/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:source/imgui/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:source/imgui/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)
