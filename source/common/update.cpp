#include <algorithm>
#include <cstdio>
#include <iterator>
#include <string>
#include <vector>

#include "impl_base.hpp"
#include "shader.hpp"
#include "update.hpp"
#include "model.hpp"

GLuint program_id;
GLuint matrix_id;
GLuint time_id;
GLuint color_id;

glm::vec3 position = glm::vec3(0.0, 2.0, 3.5);
float horizontal_angle = 3.15f;
float vertical_angle = -0.63f;
float speed = 5.0f;
float mouse_speed = 0.1f;

glm::highp_mat4 projection;
// std::vector<Sphere*> spheres;
// std::vector<Sphere*> spheres_imgui_draw_order;
// std::vector<Sphere*>::iterator spheres_mid_it;
std::vector<Model*> models;

void sort_spheres()
{
    // spheres_mid_it = std::partition(spheres.begin(), spheres.end(), [](Sphere *s){ return s->color.a == 1.0f; });
}

void graph_ops_init()
{
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    glEnable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    program_id = load_shaders();
    matrix_id = glGetUniformLocation(program_id, "MVP");
    time_id = glGetUniformLocation(program_id, "u_time");
    color_id = glGetUniformLocation(program_id, "u_color");

    // spheres = {
    //     // new Sphere(matrix_id, color_id, glm::vec4(0.0f, 1.0f, 0.0f, 0.77f), 1.0f, 64, 32),
    // };

    // sort_spheres();

    // spheres_imgui_draw_order = spheres;
    models.push_back(Model::from_obj(matrix_id, color_id, "models/test.obj"));
}

void graph_ops_update(double ticks, double dt)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program_id);

    glUniform1f(time_id, ticks);

    glm::vec3 direction =
        glm::vec3(glm::cos(vertical_angle) * glm::sin(horizontal_angle),
                  glm::sin(vertical_angle),
                  glm::cos(vertical_angle) * glm::cos(horizontal_angle));

    process_input(direction, dt);

    auto view = glm::lookAt(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
    auto view_projection = projection * view;

    // for (auto sphere_it = std::begin(spheres); sphere_it != spheres_mid_it; ++sphere_it)
    // {
    //     auto sphere = *sphere_it;
    //     sphere->draw(view_projection);
    // }

    // if (spheres_mid_it != std::end(spheres))
    // {
    //     glDepthMask(GL_FALSE);
    //     for (auto sphere_it = spheres_mid_it; sphere_it != std::end(spheres); ++sphere_it)
    //     {
    //         auto sphere = *sphere_it;
    //         sphere->draw(view_projection);
    //     }
    //     glDepthMask(GL_TRUE);
    // }

    models[0]->draw(view_projection);
}

void imgui_update()
{
    ImGui::Begin("graph-ops");

    // if (ImGui::Button("Create Sphere"))
    // {
    //     Sphere *sphere = new Sphere(matrix_id, color_id, glm::vec4(1.0f, 1.0f, 0.0f, 0.7f), 1.0f, 64, 32);
    //     spheres_imgui_draw_order.push_back(sphere);
    //     spheres.push_back(sphere);
    //     sort_spheres();
    // }

    // for (size_t i = 0; i < spheres_imgui_draw_order.size(); ++i)
    // {
    //     auto sphere = spheres_imgui_draw_order[i];
    //     auto prev_alpha = sphere->color.a;
    //     ImGui::PushID(i);
    //     ImGui::Text("Sphere %zu", i + 1);
    //     ImGui::SliderFloat("X", &sphere->model[3].x, -4.0f, 4.0f);
    //     ImGui::SliderFloat("Y", &sphere->model[3].y, -4.0f, 4.0f);
    //     ImGui::SliderFloat("Z", &sphere->model[3].z, -4.0f, 4.0f);
    //     if (ImGui::ColorPicker4("Color", (float *)&sphere->color) && sphere->color.a != prev_alpha)
    //         sort_spheres();
    //     ImGui::PopID();
    // }

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();
}
