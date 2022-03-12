# graph-ops

## Linux

    # TODO

## Windows

    clang++ -DIMGUI_USE_STB_SPRINTF imgui/*.cpp imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp source/backends/impl_glfw.cpp source/common/*.cpp -Iimgui -Iimgui/backends -Isource/include -lglfw3 -lopengl32 -lglew32
