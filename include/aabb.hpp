#pragma once

#include <vector>

#include "gl_base.hpp"

struct AABB
{
    glm::vec3 min;
    glm::vec3 max;
};

struct Box
{
    unsigned int VBO, VAO;
    std::vector<float> vertices;

    Box(AABB b);
    ~Box();

    void draw();
    void update(AABB const &b);
};

AABB calc_transformed_bounds(AABB const &b, glm::mat4 const &transform);
