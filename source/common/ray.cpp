#include "ray.hpp"

FastRay precompute_ray_inv(Ray const &ray)
{
    FastRay r;
    r.origin = ray.origin;
    r.inv_direction = glm::vec3(1.0f / ray.direction.x, 1.0f / ray.direction.y, 1.0f / ray.direction.z);
    return r;
}

// https://tavianator.com/cgit/dimension.git/tree/libdimension/bvh/bvh.c#n194
bool intersect(FastRay r, AABB b)
{
    // This is actually correct, even though it appears not to handle edge cases
    // (ray.n.{x,y,z} == 0).  It works because the infinities that result from
    // dividing by zero will still behave correctly in the comparisons.  Rays
    // which are parallel to an axis and outside the box will have tmin == inf
    // or tmax == -inf, while rays inside the box will have tmin and tmax
    // unchanged.

    float tx1 = (b.min.x - r.origin.x) * r.inv_direction.x;
    float tx2 = (b.max.x - r.origin.x) * r.inv_direction.x;

    float tmin = glm::min(tx1, tx2);
    float tmax = glm::max(tx1, tx2);

    float ty1 = (b.min.y - r.origin.y) * r.inv_direction.y;
    float ty2 = (b.max.y - r.origin.y) * r.inv_direction.y;

    tmin = glm::max(tmin, glm::min(ty1, ty2));
    tmax = glm::min(tmax, glm::max(ty1, ty2));

    float tz1 = (b.min.z - r.origin.z) * r.inv_direction.z;
    float tz2 = (b.max.z - r.origin.z) * r.inv_direction.z;

    tmin = glm::max(tmin, glm::min(tz1, tz2));
    tmax = glm::min(tmax, glm::max(tz1, tz2));

    return tmax >= glm::max(0.0f, tmin);
}

glm::vec3 cast_ray(double xpos, double ypos, int width, int height, glm::mat4 const &view, glm::mat4 const &projection)
{
    // converts a position from the 2d xpos, ypos to a normalized 3d direction
    float x = (2.0f * xpos) / width - 1.0f;
    float y = 1.0f - (2.0f * ypos) / height;
    // or (2.0f * ypos) / SCR_HEIGHT - 1.0f; depending on how you calculate ypos/lastY
    float z = 1.0f;
    glm::vec3 ray_nds = glm::vec3(x, y, z);
    glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0f, 1.0f);
    // eye space to clip we would multiply by projection so
    // clip space to eye space is the inverse projection
    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    // convert point to forwards
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
    // world space to eye space is usually multiply by view so
    // eye space to world space is inverse view
    glm::vec4 inv_ray_wor = glm::inverse(view) * ray_eye;
    glm::vec3 ray_wor = glm::vec3(inv_ray_wor.x, inv_ray_wor.y, inv_ray_wor.z);
    ray_wor = glm::normalize(ray_wor);
    return ray_wor;
}
