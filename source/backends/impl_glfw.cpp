#include <stdio.h>
#include <vector>

#include "imgui_impl_glfw.h"

#include "impl_base.hpp"
#include "io.hpp"
#include "shader.hpp"
#include "update.hpp"

int width = 1024;
int height = 768;
int g_focused = 1;

GLFWwindow *window;

extern GLuint program_id;

static void window_size_callback(GLFWwindow *window, int new_width, int new_height)
{
    (void)window;
    width = new_width;
    height = new_height;
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

void process_input(glm::vec3 &position, glm::vec3 &direction, double dt)
{
    if (g_focused)
    {
        // double xpos, ypos;
        // glfwGetCursorPos(window, &xpos, &ypos);

        // horizontal_angle += mouse_speed * dt * ((float)width / 2.0f - xpos);
        // vertical_angle += mouse_speed * dt * ((float)height / 2.0f - ypos);

        // glfwSetCursorPos(window, (float)width / 2.0f, (float)height / 2.0f);

        direction.y = 0.0f;

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

        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            position.y -= static_cast<float>(dt) * speed;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            position.y += static_cast<float>(dt) * speed;
    }
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

    window = glfwCreateWindow(width, height, "graph-ops", nullptr, nullptr);

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

    graph_ops_init();
    window_size_callback(window, width, height);

    double dt;
    double last_frame = glfwGetTime();

    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           !glfwWindowShouldClose(window))
    {
        double current_frame = glfwGetTime();
        dt = current_frame - last_frame;
        last_frame = current_frame;

#ifndef NDEBUG
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            glDeleteProgram(program_id);
            program_id = load_shaders();
        }
#endif

        graph_ops_update(current_frame, dt);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        imgui_update();

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
