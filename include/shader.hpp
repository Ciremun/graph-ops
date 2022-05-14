#pragma once

#ifdef __EMSCRIPTEN__
#include <SDL_opengles2.h>
#else
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#endif // __EMSCRIPTEN__

void compile_shader(GLuint shader_id, void *shader_source);
GLuint load_shaders(const char *vertex_path, const char *fragment_path);
