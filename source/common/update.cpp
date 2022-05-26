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
float vertical_angle = -.2f;
float speed = 5.0f;
float mouse_speed = 0.1f;
bool draw_boxes = false;

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

    program_id = load_shaders("source/shaders/color.vert.glsl", "source/shaders/color.frag.glsl");
    matrix_id = glGetUniformLocation(program_id, "u_mvp");
    time_id = glGetUniformLocation(program_id, "u_time");
    color_id = glGetUniformLocation(program_id, "u_color");

    models.push_back(Model::from_obj(matrix_id, color_id, "models/sphere.obj", "Sphere"));
    models.push_back(Model::from_obj(matrix_id, color_id, "models/link.obj", "Link"));

    selected_model = models[0];

    models[0]->texture_from_file(0);

    models[0]->color = glm::vec4(1.0f);
    models[0]->move_by(glm::vec3(0.0f, 0.0f, -2.5f));

    models[1]->move_by(glm::vec3(0.0f, -.5f, 0.0f));
    models[1]->color = glm::vec4(1.0f, 0.0f, 1.0f, 0.8f);

    Model *base = Model::from_obj(matrix_id, color_id, "models/axis_arrow.obj", "Z axis arrow");

    Model *x_axis_arrow = new Model(matrix_id, color_id, base->vertices, base->uvs, base->normals, "X axis arrow");
    x_axis_arrow->color = glm::vec4(1.0f, .0f, 0.0f, 1.0f);
    x_axis_arrow->matrix = glm::rotate(x_axis_arrow->matrix, glm::radians(270.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    x_axis_arrow->rotation.z = 270.0f;

    Model *y_axis_arrow = new Model(matrix_id, color_id, base->vertices, base->uvs, base->normals, "Y axis arrow");
    y_axis_arrow->color = glm::vec4(.0f, 1.0f, 0.0f, 1.0f);

    Model *z_axis_arrow = base;
    z_axis_arrow->color = glm::vec4(.0f, .0f, 1.0f, 1.0f);
    z_axis_arrow->matrix = glm::rotate(z_axis_arrow->matrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    z_axis_arrow->rotation.z = 90.0f;

    x_axis_arrow->box = calc_transformed_bounds(x_axis_arrow->original_box, x_axis_arrow->matrix);
    y_axis_arrow->box = calc_transformed_bounds(y_axis_arrow->original_box, y_axis_arrow->matrix);
    z_axis_arrow->box = calc_transformed_bounds(z_axis_arrow->original_box, z_axis_arrow->matrix);

    arrows.push_back(x_axis_arrow);
    arrows.push_back(y_axis_arrow);
    arrows.push_back(z_axis_arrow);

    update_model(models[0]);

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

    static const glm::vec3 up(0.0f, 1.0f, 0.0f);
    auto view = glm::lookAt(position, position + direction, up);
    auto view_projection = projection * view;

    // FIXME
    glm::vec3 prev_position = position;

    process_input(position, direction, dt);

    for (auto &model : models)
    {
        if (intersect(position - glm::vec3(.0f, .5f, .0f), model->box))
        {
            position = prev_position;
            break;
        }
    }

    prev_position = position;
    position.y -= 1.0f * dt;

    for (auto &model : models)
    {
        if (intersect(position - glm::vec3(.0f, .5f, .0f), model->box))
        {
            position = prev_position;
            break;
        }
    }

    for (auto &model : models)
    {
        if (model == selected_model)
            continue;
        glm::vec3 model_pos = glm::vec3(model->matrix[3].x, model->matrix[3].y, model->matrix[3].z);
        glm::vec3 dir = position - model_pos;
        dir.y = .0f;
        model->matrix = glm::inverse(glm::lookAt(model_pos, position, up));
        if (glm::abs(dir.x) < 1.5f && glm::abs(dir.z) < 1.5f)
            continue;
        model->move_by(glm::normalize(dir) * 2.0f * (float)dt);
    }

    if (position.y < 0.0f)
        position.y = 0.0f;

    static Model *bullet = 0;
    if (!bullet)
    {
        bullet = new Model(matrix_id, color_id, models[0]->vertices, models[0]->uvs, models[0]->normals, models[0]->label);
        bullet->matrix = glm::scale(bullet->matrix, glm::vec3(0.1f, 0.1f, 0.1f));
        bullet->color = glm::vec4(1.0f, 1.0f ,1.0f, 0.5f);
    }

    bullet->draw(view_projection);

    for (auto model_it = std::begin(models); model_it != models_mid_it; ++model_it)
    {
        glUseProgram(program_id);
        auto model = *model_it;
        model->draw(view_projection);
    }

    glUseProgram(program_id);

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
    if (draw_boxes)
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
        glm::vec3 bullet_pos(bullet->matrix[3].x, bullet->matrix[3].y, bullet->matrix[3].z);
        if (glm::abs(glm::length(position - bullet_pos)) > 6.0f)
            bullet->move_to(position + glm::normalize(casted_ray));
        else
            bullet->move_by(casted_ray / 6.0f);
    }
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
    ImGui::SetNextWindowSize(ImVec2(280.0f, 280.0f), ImGuiCond_Once);
    ImGui::Begin("graph-ops");

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
        ImGui::SetNextItemOpen(model == selected_model, ImGuiCond_Once);
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
