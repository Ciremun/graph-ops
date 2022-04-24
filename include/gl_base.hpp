#include "imgui.h"
#include "imgui_impl_opengl3.h"

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#include <SDL.h>
#include <SDL_opengles2.h>
#include <emscripten.h>
#else
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#endif // __EMSCRIPTEN__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
