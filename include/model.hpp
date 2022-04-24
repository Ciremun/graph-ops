#pragma once

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "aabb.hpp"
#include "gl_base.hpp"

struct Model
{
    GLuint matrix_id;
    GLuint color_id;
    GLuint vertex_buffer;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    glm::mat4 matrix = glm::mat4(1.0f);
    glm::vec4 color = {1.0f, 0.0f, 1.0f, 1.0f};
    glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
    AABB box;
    AABB original_box;
    const char *label;
    bool drag = false;

    Model(GLuint matrix_id, GLuint color_id, std::vector<glm::vec3> const &vertices, std::vector<glm::vec2> const &uvs, std::vector<glm::vec3> const &normals, const char *label = "");
    static Model *from_obj(GLuint matrix_id, GLuint color_id, const char *path, const char *label = "");
    void move_to(glm::vec3 const &coords);
    void move_by(glm::vec3 const &coords);
    void draw(glm::mat4 const &view_projection);
};
