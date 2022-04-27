#include <algorithm>
#include <cstdio>
#include <iterator>
#include <string>
#include <vector>

#include "aabb.hpp"
#include "impl_base.hpp"
#include "line.hpp"
#include "model.hpp"
#include "ray.hpp"
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

void update_model(Model *model)
{
    auto &xyz = model->matrix[3];
    arrows[0]->move_to(glm::vec3(xyz.x + 0.29f, xyz.y, xyz.z));
    arrows[1]->move_to(glm::vec3(xyz.x, xyz.y + 0.29f, xyz.z));
    arrows[2]->move_to(glm::vec3(xyz.x, xyz.y, xyz.z + 0.29f));
    model->box = calc_transformed_bounds(model->original_box, model->matrix);
};

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
    matrix_id = glGetUniformLocation(program_id, "u_mvp");
    time_id = glGetUniformLocation(program_id, "u_time");
    color_id = glGetUniformLocation(program_id, "u_color");

    models.push_back(Model::from_obj(matrix_id, color_id, "models/sphere.obj", "Sphere"));
    models.push_back(Model::from_obj(matrix_id, color_id, "models/link.obj", "Link"));

    models[0]->color = glm::vec4(1.0f);
    models[0]->move_by(glm::vec3(0.0f, 0.0f, -2.0f));

    models[1]->color = glm::vec4(1.0f, 0.0f, 1.0f, 0.8f);

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

    x_axis_arrow->box = calc_transformed_bounds(x_axis_arrow->original_box, x_axis_arrow->matrix);
    y_axis_arrow->box = calc_transformed_bounds(y_axis_arrow->original_box, y_axis_arrow->matrix);
    z_axis_arrow->box = calc_transformed_bounds(z_axis_arrow->original_box, z_axis_arrow->matrix);

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

    static glm::vec3 line_start(position.x, position.y, position.z);
    static glm::vec3 line_end(position.x, position.y, position.z);

    auto mvp = view_projection * glm::mat4(1.0f);
    glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvp[0][0]);
    glUniform4f(color_id, 1.0f, 1.0f, 1.0f, 1.0f);

    static Line mouse_ray_line(line_start, line_end);
    mouse_ray_line.draw();

    auto &x = arrows[0];
    auto &y = arrows[1];
    auto &z = arrows[2];
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && !ImGui::IsAnyItemActive() && !(x->drag || y->drag || z->drag))
    {
        ImVec2 xy = ImGui::GetMousePos();
        float t = 1000.0f;
        glm::vec3 casted_ray = cast_ray(xy.x, xy.y, width, height, view, projection);
        Ray mouse_ray;
        mouse_ray.origin = line_start = position;
        mouse_ray.direction = line_end = position + t * casted_ray;
        mouse_ray_line.update(line_start, line_end);
        FastRay fast_ray = precompute_ray_inv(mouse_ray);
        if (selected_model)
        {
            for (const auto &arrow : arrows)
            {
                if (intersect(fast_ray, arrow->box))
                {
                    arrow->drag = true;
                    break;
                }
            }
        }
        if (!(x->drag || y->drag || z->drag))
        {
            selected_model = NULL;
            for (const auto &model : models)
            {
                if (intersect(fast_ray, model->box))
                {
                    selected_model = model;
                    update_model(selected_model);
                    break;
                }
            }
        }
    }

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
                glm::vec3 move = {.0f, .0f, .0f};
                if (x->drag)
                {
                    if (mouse.x < prev_mouse.x)
                        move.x = -((prev_mouse.x - mouse.x) / 140.0f);
                    else if (mouse.x > prev_mouse.x)
                        move.x = (mouse.x - prev_mouse.x) / 140.0f;
                }
                if (y->drag)
                {
                    if (mouse.y < prev_mouse.y)
                        move.y = (prev_mouse.y - mouse.y) / 140.0f;
                    else if (mouse.y > prev_mouse.y)
                        move.y = -((mouse.y - prev_mouse.y) / 140.0f);
                }
                if (z->drag)
                {
                    if (mouse.y < prev_mouse.y)
                        move.z = -((prev_mouse.y - mouse.y) / 140.0f);
                    else if (mouse.y > prev_mouse.y)
                        move.z = (mouse.y - prev_mouse.y) / 140.0f;
                }
                selected_model->move_by(move);
                for (auto &arrow : arrows)
                    arrow->move_by(move);
            }
        }
        prev_mouse = mouse;
    }
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        for (const auto &arrow : arrows)
            arrow->drag = false;
    }

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(260.0f, 260.0f), ImGuiCond_Once);
    ImGui::Begin("graph-ops");

    static bool draw_boxes = false;
    ImGui::Checkbox("Draw Boxes", &draw_boxes);
    if (draw_boxes)
    {
        if (selected_model)
        {
            static Box model_box(selected_model->box);
            model_box.update(selected_model->box);
            model_box.draw();
        }

        static Box x_arrow(arrows[0]->box);
        x_arrow.update(arrows[0]->box);
        x_arrow.draw();

        static Box y_arrow(arrows[1]->box);
        y_arrow.update(arrows[1]->box);
        y_arrow.draw();

        static Box z_arrow(arrows[2]->box);
        z_arrow.update(arrows[2]->box);
        z_arrow.draw();
    }

    if (ImGui::Button("Copy Selected Model"))
    {
        const auto &base = selected_model ? selected_model : models[0];
        Model *model = new Model(matrix_id, color_id, base->vertices, base->uvs, base->normals, base->label);
        models_imgui_draw_order.push_back(model);
        models.push_back(model);
        sort_models();
    }

    if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_None))
    {
        ImGui::SliderFloat("Pos X", &position.x, -4.0f, 4.0f);
        ImGui::SliderFloat("Pos Y", &position.y, -4.0f, 4.0f);
        ImGui::SliderFloat("Pos Z", &position.z, -4.0f, 4.0f);
        ImGui::SliderFloat("H", &horizontal_angle, -4.0f, 4.0f);
        ImGui::SliderFloat("V", &vertical_angle, -4.0f, 4.0f);
    }

    for (size_t i = 0; i < models_imgui_draw_order.size(); ++i)
    {
        auto &model = models_imgui_draw_order[i];
        ImGui::PushID(i);
        if (i == 0)
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader(model->label, ImGuiTreeNodeFlags_None))
        {
            if (ImGui::SliderFloat("X", &model->matrix[3].x, -24.0f, 24.0f))
                update_model(model);
            if (ImGui::SliderFloat("Y", &model->matrix[3].y, -24.0f, 24.0f))
                update_model(model);
            if (ImGui::SliderFloat("Z", &model->matrix[3].z, -24.0f, 24.0f))
                update_model(model);
            float prev_x_rotation = model->rotation.x;
            float prev_y_rotation = model->rotation.y;
            float prev_z_rotation = model->rotation.z;
            if (ImGui::SliderFloat("Xr", &model->rotation.x, .0f, 360.0f))
            {
                if (model->rotation.x != prev_x_rotation)
                {
                    float radians = model->rotation.x < prev_x_rotation ? glm::radians(-(prev_x_rotation - model->rotation.x)) : glm::radians(model->rotation.x - prev_x_rotation);
                    model->matrix = glm::rotate(model->matrix, radians, glm::vec3(1.0f, 0.0f, 0.0f));
                    model->box = calc_transformed_bounds(model->original_box, model->matrix);
                }
            }
            if (ImGui::SliderFloat("Yr", &model->rotation.y, .0f, 360.0f))
            {
                if (model->rotation.y != prev_y_rotation)
                {
                    float radians = model->rotation.y < prev_y_rotation ? glm::radians(-(prev_y_rotation - model->rotation.y)) : glm::radians(model->rotation.y - prev_y_rotation);
                    model->matrix = glm::rotate(model->matrix, radians, glm::vec3(0.0f, 1.0f, 0.0f));
                    model->box = calc_transformed_bounds(model->original_box, model->matrix);
                }
            }
            if (ImGui::SliderFloat("Zr", &model->rotation.z, .0f, 360.0f))
            {
                if (model->rotation.z != prev_z_rotation)
                {
                    float radians = model->rotation.z < prev_z_rotation ? glm::radians(-(prev_z_rotation - model->rotation.z)) : glm::radians(model->rotation.z - prev_z_rotation);
                    model->matrix = glm::rotate(model->matrix, radians, glm::vec3(0.0f, 0.0f, 1.0f));
                    model->box = calc_transformed_bounds(model->original_box, model->matrix);
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
