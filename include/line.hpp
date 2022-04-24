#pragma once

#include <vector>

#include "gl_base.hpp"

struct Line
{
    unsigned int VBO, VAO;
    std::vector<float> vertices;

    Line(glm::vec3 start, glm::vec3 end);
    ~Line();

    void draw();
    void update(glm::vec3 const &start, glm::vec3 const &end);
};
