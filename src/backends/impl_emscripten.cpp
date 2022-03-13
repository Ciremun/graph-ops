#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"
#include <SDL.h>
#include <SDL_opengles2.h>
#include <emscripten.h>
#include <stdio.h>

#include "shader.hpp"
#include "sphere.hpp"

EM_JS(int, canvas_get_width, (), {
    return window.canvas.width;
});

EM_JS(int, canvas_get_height, (), {
    return window.canvas.height;
});

SDL_Window *g_Window = NULL;
SDL_GLContext g_GLContext = NULL;

glm::highp_mat4 projection;

static void main_loop(void *);

static void window_size_callback(int width, int height)
{
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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
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
    (void)io;

    io.IniFilename = NULL;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(g_Window, g_GLContext);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImFontConfig font_cfg;
    font_cfg.SizePixels = 22.0f;
    io.Fonts->AddFontDefault(&font_cfg);

#ifndef IMGUI_DISABLE_FILE_FUNCTIONS
    io.Fonts->AddFontFromFileTTF("imgui/fonts/Roboto-Medium.ttf", 16.0f);
#endif

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glDepthFunc(GL_LESS);

    window_size_callback(width, height);

    emscripten_set_main_loop_arg(main_loop, NULL, 0, true);
}

static void main_loop(void *arg)
{
    static GLuint program_id = load_shaders();
    static GLuint matrix_id = glGetUniformLocation(program_id, "MVP");
    static GLuint time_id = glGetUniformLocation(program_id, "u_time");
    static GLuint color_id = glGetUniformLocation(program_id, "u_color");

    static auto position = glm::vec3(0.0, 2.0, 3.5);

    static float horizontal_angle = 3.15f;
    static float vertical_angle = -0.63f;
    static float speed = 5.0f;
    static float mouse_speed = 0.1f;

    static std::vector<Sphere> spheres = {
        {matrix_id, color_id, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), 1.0f, 64, 32},
    };

    ImGuiIO &io = ImGui::GetIO();
    IM_UNUSED(arg);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            window_size_callback(event.window.data1, event.window.data2);
        ImGui_ImplSDL2_ProcessEvent(&event);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program_id);

    auto direction =
        glm::vec3(glm::cos(vertical_angle) * glm::sin(horizontal_angle),
                  glm::sin(vertical_angle),
                  glm::cos(vertical_angle) * glm::cos(horizontal_angle));

    auto view = glm::lookAt(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
    auto view_projection = projection * view;

    glUniform1f(time_id, SDL_GetTicks64() / 1000.0f);

    for (const auto &sphere : spheres)
        sphere.draw(view_projection);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("graph-ops");

    if (ImGui::Button("Create Sphere"))
        spheres.push_back(Sphere(matrix_id, color_id, glm::vec4(1.0f, 1.0f, 0.0f, 0.7f), 1.0f, 64, 32));

    for (size_t i = 0; i < spheres.size(); ++i)
    {
        ImGui::PushID(i);
        ImGui::Text("Sphere %llu", i + 1);
        ImGui::SliderFloat("X", &spheres[i].model[3][0], -4.0f, 4.0f);
        ImGui::SliderFloat("Y", &spheres[i].model[3][1], -4.0f, 4.0f);
        ImGui::SliderFloat("Z", &spheres[i].model[3][2], -4.0f, 4.0f);
        ImGui::ColorPicker4("Color", (float *)&spheres[i].color);
        ImGui::PopID();
    }

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();

    ImGui::Render();
    SDL_GL_MakeCurrent(g_Window, g_GLContext);
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(g_Window);
}
