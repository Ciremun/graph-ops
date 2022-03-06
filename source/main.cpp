#include <stdio.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "io.hpp"
#include "sphere.h"

#define WIDTH 1024
#define HEIGHT 768

#define PANIC(fmt, ...)                                        \
    do                                                         \
    {                                                          \
        printf("ERROR: %d: " fmt "\n", __LINE__, __VA_ARGS__); \
        exit(1);                                               \
    } while (0)

int g_focused = 1;
double g_cursor_xpos = .0f;
double g_cursor_ypos = .0f;

void window_size_callback(GLFWwindow* window, int width, int height)
{
    (void)window;
    glViewport(0, 0, width, height);
}

void window_focus_callback(GLFWwindow* window, int focused)
{
    (void)window;
    g_focused = focused;
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    (void)window;
    g_cursor_xpos = xpos;
    g_cursor_ypos = ypos;
}

void compile_shader(GLuint shader_id, void *shader_source)
{
    GLint compiled = false;

    glShaderSource(shader_id, 1, (const GLchar *const *)&shader_source, nullptr);
    glCompileShader(shader_id);
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled);

    if (compiled != GL_TRUE)
    {
        GLchar buffer[1024] = {0};
        GLsizei length = 0;
        glGetShaderInfoLog(shader_id, sizeof(buffer), &length, buffer);
        PANIC("Could not compile shader: %s", buffer);
    }
}

GLuint load_shaders()
{
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

#if 0
compile_shader(vertex_shader_id, VERTEX_SHADER_SOURCE);
compile_shader(fragment_shader_id, FRAGMENT_SHADER_SOURCE);
#else
    auto vertex_shader_file =
        open_and_map_file("source/shaders/vert.glsl", IO_READ_ONLY);
    compile_shader(vertex_shader_id, vertex_shader_file.start);
    UNMAP_AND_CLOSE_FILE(vertex_shader_file);

    auto fragment_shader_file =
        open_and_map_file("source/shaders/frag.glsl", IO_READ_ONLY);
    compile_shader(fragment_shader_id, fragment_shader_file.start);
    UNMAP_AND_CLOSE_FILE(fragment_shader_file);
#endif // NDEBUG

    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    glLinkProgram(program_id);

    GLint linked = 0;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked);

    if (linked != GL_TRUE)
    {
        GLchar buffer[1024] = {0};
        GLsizei length = 0;
        glGetProgramInfoLog(program_id, sizeof(buffer), &length, buffer);
        PANIC("Could not compile shader: %s", buffer);
    }

    glDetachShader(program_id, vertex_shader_id);
    glDetachShader(program_id, fragment_shader_id);

    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    return program_id;
}

int main()
{
    if (!glfwInit())
        return 1;

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow *window =
        glfwCreateWindow(WIDTH, HEIGHT, "graph-ops", nullptr, nullptr);

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
    glfwSetCursorPosCallback(window, cursor_position_callback);

    printf("%s\n", glGetString(GL_VERSION));

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glDepthFunc(GL_LESS);

    GLuint vertex_array_id;
    glGenVertexArrays(1, &vertex_array_id);
    glBindVertexArray(vertex_array_id);

    GLuint program_id = load_shaders();

    GLuint matrix_id = glGetUniformLocation(program_id, "MVP");
    GLuint time_id = glGetUniformLocation(program_id, "u_time");

    auto projection =
        glm::perspective(glm::radians(90.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    auto model = glm::mat4(1.0f);

    auto position = glm::vec3(0.0, 2.0, 3.5);

    float horizontal_angle = 3.15f;
    float vertical_angle = -0.63f;
    float speed = 5.0f;
    float mouse_speed = 0.1f;

    double dt;
    double last_frame = glfwGetTime();

    static const GLfloat g_vertex_buffer_data[] = {
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f};

    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data),
                 g_vertex_buffer_data, GL_STATIC_DRAW);

    // create a sphere with default params; radius=1, sectors=36, stacks=18,
    // smooth=true
    Sphere sphere(1, 64, 32, true);

    // copy interleaved vertex data (V/N/T) to VBO
    GLuint vboId;
    glGenBuffers(1, &vboId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);           // for vertex data
    glBufferData(GL_ARRAY_BUFFER,                   // target
                 sphere.getInterleavedVertexSize(), // data size, # of bytes
                 sphere.getInterleavedVertices(),   // ptr to vertex data
                 GL_STATIC_DRAW);                   // usage

    // copy index data to VBO
    GLuint iboId;
    glGenBuffers(1, &iboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId); // for index data
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,         // target
                 sphere.getIndexSize(),           // data size, # of bytes
                 sphere.getIndices(),             // ptr to index data
                 GL_STATIC_DRAW);                 // usage

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
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            horizontal_angle += mouse_speed * dt * ((float)WIDTH / 2.0f - xpos);
            vertical_angle += mouse_speed * dt * ((float)HEIGHT / 2.0f - ypos);

            auto right = glm::vec3(glm::sin(horizontal_angle - 3.14f / 2.0), 0.0f,
                                   glm::cos(horizontal_angle - 3.14 / 2.0));

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                position = position + direction * static_cast<float>(dt) * speed;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                position = position - direction * static_cast<float>(dt) * speed;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                position = position + right * static_cast<float>(dt) * speed;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                position = position - right * static_cast<float>(dt) * speed;
            glfwSetCursorPos(window, (float)WIDTH / 2.0f, (float)HEIGHT / 2.0f);
        }

        auto view = glm::lookAt(position, position + direction,
                                glm::vec3(0.0f, 1.0f, 0.0f));
        auto view_projection = projection * view;
        auto mvp = view_projection * model;

        static const auto right_model =
            glm::scale(glm::translate(model, glm::vec3(2.0, 0.0, 0.0)),
                       glm::vec3(0.3, 0.3, 0.3));

        auto mvp_2 = view_projection * right_model;
        glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvp_2[0][0]);
        glUniform1f(time_id, current_frame);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glDrawArrays(GL_TRIANGLES, 0, 12 * 3);

        glDisableVertexAttribArray(0);

        glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvp[0][0]);

        // bind VBOs
        glBindBuffer(GL_ARRAY_BUFFER, vboId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);

        // activate attrib arrays
        glEnableVertexAttribArray(0);

        // set attrib arrays with stride and offset
        int stride = sphere.getInterleavedStride(); // should be 32 bytes
        glVertexAttribPointer(0, 3, GL_FLOAT, false, stride, (void *)0);

        // draw a sphere with VBO
        glDrawElements(GL_TRIANGLES,           // primitive type
                       sphere.getIndexCount(), // # of indices
                       GL_UNSIGNED_INT,        // data type
                       (void *)0);             // offset to indices

        // deactivate attrib arrays
        glDisableVertexAttribArray(0);

        // unbind VBOs
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
