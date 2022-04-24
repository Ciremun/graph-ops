#include "aabb.hpp"

AABB calc_transformed_bounds(AABB const &b, glm::mat4 const &transform)
{
    const std::vector<glm::vec3> corners = {
        b.min,
        {b.min.x, b.min.y, b.max.z},
        {b.min.x, b.max.y, b.min.z},
        {b.max.x, b.min.y, b.min.z},
        {b.min.x, b.max.y, b.max.z},
        {b.max.x, b.min.y, b.max.z},
        {b.max.x, b.max.y, b.min.z},
        b.max,
    };

    glm::vec3 min = {INFINITY, INFINITY, INFINITY};
    glm::vec3 max = {-INFINITY, -INFINITY, -INFINITY};

    for (int i = 0; i < 8; i++)
    {
        glm::vec3 transformed = glm::vec3(transform * glm::vec4(corners[i], 1.0f));
        min = glm::min(min, transformed);
        max = glm::max(max, transformed);
    }

    AABB box;
    box.min = min;
    box.max = max;

    return box;
}

Box::Box(AABB b)
{
    vertices = {
        b.min.x,
        b.min.y,
        b.min.z,
        b.max.x,
        b.min.y,
        b.min.z,
        b.max.x,
        b.max.y,
        b.min.z,
        b.min.x,
        b.max.y,
        b.min.z,

        b.min.x,
        b.min.y,
        b.max.z,
        b.max.x,
        b.min.y,
        b.max.z,
        b.max.x,
        b.max.y,
        b.max.z,
        b.min.x,
        b.max.y,
        b.max.z,

        b.min.x,
        b.min.y,
        b.min.z,
        b.min.x,
        b.min.y,
        b.max.z,

        b.max.x,
        b.min.y,
        b.min.z,
        b.max.x,
        b.min.y,
        b.max.z,

        b.max.x,
        b.max.y,
        b.min.z,
        b.max.x,
        b.max.y,
        b.max.z,

        b.min.x,
        b.max.y,
        b.min.z,
        b.min.x,
        b.max.y,
        b.max.z,
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Box::update(AABB const &b)
{
    vertices = {
        b.min.x,
        b.min.y,
        b.min.z,
        b.max.x,
        b.min.y,
        b.min.z,
        b.max.x,
        b.max.y,
        b.min.z,
        b.min.x,
        b.max.y,
        b.min.z,

        b.min.x,
        b.min.y,
        b.max.z,
        b.max.x,
        b.min.y,
        b.max.z,
        b.max.x,
        b.max.y,
        b.max.z,
        b.min.x,
        b.max.y,
        b.max.z,

        b.min.x,
        b.min.y,
        b.min.z,
        b.min.x,
        b.min.y,
        b.max.z,

        b.max.x,
        b.min.y,
        b.min.z,
        b.max.x,
        b.min.y,
        b.max.z,

        b.max.x,
        b.max.y,
        b.min.z,
        b.max.x,
        b.max.y,
        b.max.z,

        b.min.x,
        b.max.y,
        b.min.z,
        b.min.x,
        b.max.y,
        b.max.z,
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
}

void Box::draw()
{
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glDrawArrays(GL_LINE_LOOP, 4, 4);
    glDrawArrays(GL_LINES, 8, 8);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Box::~Box()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}
