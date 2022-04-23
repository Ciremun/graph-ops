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
Model *selected_model = NULL;

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
    x_axis_arrow->matrix = glm::rotate(x_axis_arrow->matrix, glm::radians(270.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    x_axis_arrow->rotation.z = 270.0f;
    x_axis_arrow->matrix[3].x = 0.29f;

    Model *y_axis_arrow = new Model(matrix_id, color_id, base->vertices, base->uvs, base->normals, "Y axis arrow");
    y_axis_arrow->color = glm::vec4(.0f, 1.0f, 0.0f, 1.0f);
    y_axis_arrow->matrix[3].y = 0.29f;

    Model *z_axis_arrow = base;
    z_axis_arrow->color = glm::vec4(.0f, .0f, 1.0f, 1.0f);
    z_axis_arrow->matrix = glm::rotate(z_axis_arrow->matrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    z_axis_arrow->rotation.z = 90.0f;
    z_axis_arrow->matrix[3].z = 0.29f;

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

    glClear(GL_DEPTH_BUFFER_BIT);

    if (selected_model)
        for (const auto &arrow : arrows)
            arrow->draw(view_projection);

    process_input(position, direction, dt);
}

void imgui_update()
{
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        static ImVec2 prev_mouse(0.0f, 0.0f);
        ImVec2 mouse = ImGui::GetMousePos();
        if (mouse.x != prev_mouse.x || mouse.y != prev_mouse.y)
        {
            auto &x = arrows[0];
            auto &y = arrows[1];
            auto &z = arrows[2];
            if (selected_model && (x->drag || y->drag || z->drag))
            {
                float move_x = 0.0f;
                float move_y = 0.0f;
                float move_z = 0.0f;
                if (x->drag)
                {
                    if (mouse.x < prev_mouse.x)
                        move_x = -((prev_mouse.x - mouse.x) / 140.0f);
                    else if (mouse.x > prev_mouse.x)
                        move_x = (mouse.x - prev_mouse.x) / 140.0f;
                }
                if (y->drag)
                {
                    if (mouse.y < prev_mouse.y)
                        move_y = (prev_mouse.y - mouse.y) / 140.0f;
                    else if (mouse.y > prev_mouse.y)
                        move_y = -((mouse.y - prev_mouse.y) / 140.0f);
                }
                if (z->drag)
                {
                    if (mouse.y < prev_mouse.y)
                        move_z = -((prev_mouse.y - mouse.y) / 140.0f);
                    else if (mouse.y > prev_mouse.y)
                        move_z = (mouse.y - prev_mouse.y) / 140.0f;
                }
                selected_model->matrix[3].x += move_x;
                selected_model->matrix[3].y += move_y;
                selected_model->matrix[3].z += move_z;
                for (auto & arrow: arrows)
                {
                    arrow->matrix[3].x += move_x;
                    arrow->matrix[3].y += move_y;
                    arrow->matrix[3].z += move_z;
                }
            }
            else
            {
                unsigned char data[4];
                glReadPixels(mouse.x, height - mouse.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
                if (selected_model)
                {
                    if (data[0] == 255 && data[1] == 0 && data[2] == 0 && data[3] == 255)
                        x->drag = true;
                    else if (data[0] == 0 && data[1] == 255 && data[2] == 0 && data[3] == 255)
                        y->drag = true;
                    else if (data[0] == 0 && data[1] == 0 && data[2] == 255 && data[3] == 255)
                        z->drag = true;
                    else if (!(data[0] == 255 && data[1] == 0 && data[2] == 255 && data[3] == 255))
                        selected_model = NULL;
                }
                else if (data[0] == 255 && data[1] == 0 && data[2] == 255 && data[3] == 255)
                    selected_model = models[0];
            }
        }
        prev_mouse = mouse;
    }
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        for (const auto &arrow : arrows)
            arrow->drag = false;
    }

    ImGui::SetNextWindowSize(ImVec2(0.0f, 520.0f), ImGuiCond_Once);
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
        if (i == 0)
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader(model->label, ImGuiTreeNodeFlags_None))
        {
            ImGui::SliderFloat("X", &model->matrix[3].x, -24.0f, 24.0f);
            ImGui::SliderFloat("Y", &model->matrix[3].y, -24.0f, 24.0f);
            ImGui::SliderFloat("Z", &model->matrix[3].z, -24.0f, 24.0f);
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
