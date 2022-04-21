#include <algorithm>
#include <cstdio>
#include <iterator>
#include <string>
#include <vector>

#include "impl_base.hpp"
#include "model.hpp"
#include "shader.hpp"
#include "update.hpp"

GLuint program_id;
GLuint matrix_id;
GLuint time_id;
GLuint color_id;

glm::vec3 position = glm::vec3(.0f, 2.0f, 2.0f);
float horizontal_angle = 3.15f;
float vertical_angle = -0.63f;
float speed = 5.0f;
float mouse_speed = 0.1f;

glm::highp_mat4 projection;
std::vector<Model *> models_imgui_draw_order;
std::vector<Model *>::iterator models_mid_it;
std::vector<Model *> models;
std::vector<Model *> arrows;

void sort_models()
{
    models_mid_it = std::partition(models.begin(), models.end(), [](Model *m)
                                   { return m->color.a == 1.0f; });
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

    models.push_back(Model::from_obj(matrix_id, color_id, "models/link.obj", "Link"));

    Model *base = Model::from_obj(matrix_id, color_id, "models/axis_arrow.obj", "Z axis arrow");

    Model *x_axis_arrow = new Model(matrix_id, color_id, base->vertices, base->uvs, base->normals, "X axis arrow");
    x_axis_arrow->color = glm::vec4(1.0f, .0f, 0.0f, 1.0f);

    Model *y_axis_arrow = new Model(matrix_id, color_id, base->vertices, base->uvs, base->normals, "Y axis arrow");
    y_axis_arrow->color = glm::vec4(.0f, 1.0f, 0.0f, 1.0f);

    Model *z_axis_arrow = base;
    z_axis_arrow->color = glm::vec4(.0f, .0f, 1.0f, 1.0f);

    models.push_back(x_axis_arrow);
    models.push_back(y_axis_arrow);
    models.push_back(z_axis_arrow);

    arrows.push_back(x_axis_arrow);
    arrows.push_back(y_axis_arrow);
    arrows.push_back(z_axis_arrow);

    sort_models();
    models_imgui_draw_order = models;
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

    auto view = glm::lookAt(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
    auto view_projection = projection * view;

    for (auto model_it = std::begin(models); model_it != models_mid_it; ++model_it)
    {
        auto model = *model_it;
        model->draw(view_projection);
    }

    if (models_mid_it != std::end(models))
    {
        glDepthMask(GL_FALSE);
        for (auto model_it = models_mid_it; model_it != std::end(models); ++model_it)
        {
            auto model = *model_it;
            model->draw(view_projection);
        }
        glDepthMask(GL_TRUE);
    }

    process_input(position, direction, dt);
}

void imgui_update()
{
    ImGui::Begin("graph-ops");

    if (ImGui::Button("Create Model"))
    {
        const auto &base = models[0];
        Model *model = new Model(matrix_id, color_id, base->vertices, base->uvs, base->normals);
        models_imgui_draw_order.push_back(model);
        models.push_back(model);
        sort_models();
    }

    for (size_t i = 0; i < models_imgui_draw_order.size(); ++i)
    {
        auto &model = models_imgui_draw_order[i];
        ImGui::PushID(i);
        if (ImGui::CollapsingHeader(model->label, ImGuiTreeNodeFlags_None))
        {
            ImGui::SliderFloat("X", &model->matrix[3].x, -1024.0f, 1024.0f);
            ImGui::SliderFloat("Y", &model->matrix[3].y, -1024.0f, 1024.0f);
            ImGui::SliderFloat("Z", &model->matrix[3].z, -1024.0f, 1024.0f);
            float prev_x_rotation = model->rotation.x;
            float prev_y_rotation = model->rotation.y;
            float prev_z_rotation = model->rotation.z;
            if (ImGui::SliderFloat("Xr", &model->rotation.x, .0f, 360.0f))
            {
                if (model->rotation.x != prev_x_rotation)
                {
                    float radians = model->rotation.x < prev_x_rotation ? glm::radians(-(prev_x_rotation - model->rotation.x)) : glm::radians(model->rotation.x - prev_x_rotation);
                    model->matrix = glm::rotate(model->matrix, radians, glm::vec3(1.0f, 0.0f, 0.0f));
                }
            }
            if (ImGui::SliderFloat("Yr", &model->rotation.y, .0f, 360.0f))
            {
                if (model->rotation.y != prev_y_rotation)
                {
                    float radians = model->rotation.y < prev_y_rotation ? glm::radians(-(prev_y_rotation - model->rotation.y)) : glm::radians(model->rotation.y - prev_y_rotation);
                    model->matrix = glm::rotate(model->matrix, radians, glm::vec3(0.0f, 1.0f, 0.0f));
                }
            }
            if (ImGui::SliderFloat("Zr", &model->rotation.z, .0f, 360.0f))
            {
                if (model->rotation.z != prev_z_rotation)
                {
                    float radians = model->rotation.z < prev_z_rotation ? glm::radians(-(prev_z_rotation - model->rotation.z)) : glm::radians(model->rotation.z - prev_z_rotation);
                    model->matrix = glm::rotate(model->matrix, radians, glm::vec3(0.0f, 0.0f, 1.0f));
                }
            }
            auto prev_alpha = model->color.a;
            if (ImGui::ColorPicker4("Color", (float *)&model->color) && model->color.a != prev_alpha)
                sort_models();
        }
        ImGui::PopID();
    }

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();
}
