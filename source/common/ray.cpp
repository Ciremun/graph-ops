#include "ray.hpp"

bool intersect(Ray r, AABB b)
{
    // r.direction is unit direction vector of ray
    glm::vec3 dirfrac;
    dirfrac.x = 1.0f / r.direction.x;
    dirfrac.y = 1.0f / r.direction.y;
    dirfrac.z = 1.0f / r.direction.z;
    // lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
    // r.origin is origin of ray
    float t1 = (b.min.x - r.origin.x) * dirfrac.x;
    float t2 = (b.max.x - r.origin.x) * dirfrac.x;
    float t3 = (b.min.y - r.origin.y) * dirfrac.y;
    float t4 = (b.max.y - r.origin.y) * dirfrac.y;
    float t5 = (b.min.z - r.origin.z) * dirfrac.z;
    float t6 = (b.max.z - r.origin.z) * dirfrac.z;

    float tmin = glm::max(glm::max(glm::min(t1, t2), glm::min(t3, t4)), glm::min(t5, t6));
    float tmax = glm::min(glm::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

    // if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
    if (tmax < 0)
    {
        // t = tmax;
        return false;
    }

    // if tmin > tmax, ray doesn't intersect AABB
    if (tmin > tmax)
    {
        // t = tmax;
        return false;
    }

    // t = tmin;
    return true;
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
