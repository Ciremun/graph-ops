#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdio.h>
#include <vector>

#include "io.hpp"
#include "sphere.hpp"
#include "shader.hpp"

int width = 1024;
int height = 768;
int g_focused = 1;

glm::highp_mat4 projection;

static void window_size_callback(GLFWwindow *window, int width, int height)
{
    (void)window;
    glViewport(0, 0, width, height);
    projection = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 100.0f);
}

static void window_focus_callback(GLFWwindow *window, int focused)
{
    (void)window;
    g_focused = focused;
}

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

#if defined(IMGUI_IMPL_OPENGL_ES2)
    const char *glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    const char *glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    GLFWwindow *window =
        glfwCreateWindow(width, height, "graph-ops", nullptr, nullptr);

    if (!window)
    {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = true;
    if (glewInit() != GLEW_OK)
        return 1;

    glfwSwapInterval(-1);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetWindowFocusCallback(window, window_focus_callback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.IniFilename = NULL;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    printf("%s\n", glGetString(GL_VERSION));

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glDepthFunc(GL_LESS);

    GLuint program_id = load_shaders();
    GLuint matrix_id = glGetUniformLocation(program_id, "MVP");
    GLuint time_id = glGetUniformLocation(program_id, "u_time");
    GLuint color_id = glGetUniformLocation(program_id, "u_color");

    // set projection
    window_size_callback(window, width, height);
    auto position = glm::vec3(0.0, 2.0, 3.5);

    float horizontal_angle = 3.15f;
    float vertical_angle = -0.63f;
    float speed = 5.0f;

    double dt;
    double last_frame = glfwGetTime();

    std::vector<Sphere> spheres = {
        {matrix_id, color_id, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), 1.0f, 64, 32},
    };

    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           !glfwWindowShouldClose(window))
    {
        double current_frame = glfwGetTime();
        dt = current_frame - last_frame;
        last_frame = current_frame;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifndef NDEBUG
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            glDeleteProgram(program_id);
            program_id = load_shaders();
        }
#endif

        glUseProgram(program_id);

        auto direction =
            glm::vec3(glm::cos(vertical_angle) * glm::sin(horizontal_angle),
                      glm::sin(vertical_angle),
                      glm::cos(vertical_angle) * glm::cos(horizontal_angle));

        if (g_focused)
        {
            // double xpos, ypos;
            // glfwGetCursorPos(window, &xpos, &ypos);

            // horizontal_angle += mouse_speed * dt * ((float)WIDTH / 2.0f - xpos);
            // vertical_angle += mouse_speed * dt * ((float)HEIGHT / 2.0f - ypos);

            auto right = glm::vec3(glm::sin(horizontal_angle - 3.14 / 2.0), 0.0f,
                                   glm::cos(horizontal_angle - 3.14 / 2.0));

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                position = position + direction * static_cast<float>(dt) * speed;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                position = position - direction * static_cast<float>(dt) * speed;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                position = position + right * static_cast<float>(dt) * speed;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                position = position - right * static_cast<float>(dt) * speed;

            // glfwSetCursorPos(window, (float)WIDTH / 2.0f, (float)HEIGHT / 2.0f);
        }

        auto view = glm::lookAt(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
        auto view_projection = projection * view;

        glUniform1f(time_id, current_frame);

        for (const auto &sphere : spheres)
            sphere.draw(view_projection);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
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
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
