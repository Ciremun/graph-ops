#pragma once

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "gl_base.hpp"

struct aabb
{
    glm::vec3 min;
    glm::vec3 max;
};

struct ray
{
    glm::vec3 org;
    glm::vec3 dir;
};

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
    aabb box;
    aabb box_copy;
    const char *label;
    bool drag = false;

    Model(GLuint matrix_id, GLuint color_id, std::vector<glm::vec3> const &vertices, std::vector<glm::vec2> const &uvs, std::vector<glm::vec3> const &normals, const char *label = "");
    void draw(glm::mat4 const &view_projection);
    static Model *from_obj(GLuint matrix_id, GLuint color_id, const char *path, const char *label = "");
};
