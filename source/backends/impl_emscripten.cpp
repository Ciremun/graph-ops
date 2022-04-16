#include "imgui_impl_sdl.h"
#include <SDL.h>
#include <SDL_opengles2.h>
#include <emscripten.h>
#include <stdio.h>

#include "impl_base.hpp"
#include "shader.hpp"
#include "update.hpp"

EM_JS(int, canvas_get_width, (), {
    return window.canvas.width;
});

EM_JS(int, canvas_get_height, (), {
    return window.canvas.height;
});

EM_JS(int, mobile, (), {
    return navigator.userAgentData.mobile;
});

SDL_Window *g_Window = NULL;
SDL_GLContext g_GLContext = NULL;

void process_input(glm::vec3 const &direction, double dt)
{
    (void)direction;
    (void)dt;
}

static void main_loop(void *);

static void window_size_callback(int width, int height)
{
    if (!mobile())
        glViewport(0, 0, width, height);
    projection = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 100.0f);
}

int main(int, char **)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    const char *glsl_version = "#version 300 es";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    int width = canvas_get_width();
    int height = canvas_get_height();
    g_Window = SDL_CreateWindow("graph-ops", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, window_flags);
    g_GLContext = SDL_GL_CreateContext(g_Window);
    if (!g_GLContext)
    {
        fprintf(stderr, "Failed to initialize WebGL context!\n");
        return 1;
    }
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(g_Window, g_GLContext);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImFontConfig font_cfg;
    font_cfg.SizePixels = 22.0f;
    io.Fonts->AddFontDefault(&font_cfg);

#ifndef IMGUI_DISABLE_FILE_FUNCTIONS
    io.Fonts->AddFontFromFileTTF("source/imgui/fonts/Roboto-Medium.ttf", 16.0f);
#endif

    graph_ops_init();
    window_size_callback(width, height);

    emscripten_set_main_loop_arg(main_loop, NULL, 0, true);
}

static void main_loop(void *arg)
{
    IM_UNUSED(arg);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            window_size_callback(event.window.data1, event.window.data2);
        ImGui_ImplSDL2_ProcessEvent(&event);
    }

    graph_ops_update(SDL_GetTicks64() / 1000.0f, NULL);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    imgui_update();

    ImGui::Render();
    SDL_GL_MakeCurrent(g_Window, g_GLContext);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(g_Window);
}