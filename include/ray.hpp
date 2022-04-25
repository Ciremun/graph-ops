#pragma once

#include "gl_base.hpp"

#include "aabb.hpp"

struct Ray
{
    glm::vec3 origin;
    glm::vec3 direction;
};

struct FastRay
{
    glm::vec3 origin;
    glm::vec3 inv_direction;
};

bool intersect(FastRay r, AABB b);
FastRay precompute_ray_inv(Ray const &ray);
glm::vec3 cast_ray(double xpos, double ypos, int width, int height, glm::mat4 const &view, glm::mat4 const &projection);
