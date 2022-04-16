#include "io.hpp"
#include "shader.hpp"

#define PANIC(fmt, ...)                                        \
    do                                                         \
    {                                                          \
        printf("ERROR: %d: " fmt "\n", __LINE__, __VA_ARGS__); \
        exit(1);                                               \
    } while (0)

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
