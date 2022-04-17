#pragma once

#include <cstdlib>
#include <string>
#include <cstdio>
#include <vector>

#include "gl_base.hpp"

struct Model
{
    GLuint matrix_id;
    GLuint color_id;
    GLuint vbo_id;
    GLuint ibo_id;
    GLuint vertex_buffer;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    glm::mat4 matrix;
    glm::vec4 color;

    Model(std::vector<glm::vec3> const &vertices, std::vector<glm::vec2> const &uvs, std::vector<glm::vec3> const &normals);
    void draw(glm::mat4 const &view_projection);
    static Model* from_obj(const char *path);
};
