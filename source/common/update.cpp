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

aabb GetTransformedBounds(aabb original, glm::mat4 transformation)
{
    std::vector<glm::vec3> corners = {
        original.min,
        {original.min.x, original.min.y, original.max.z},
        {original.min.x, original.max.y, original.min.z},
        {original.max.x, original.min.y, original.min.z},
        {original.min.x, original.max.y, original.max.z},
        {original.max.x, original.min.y, original.max.z},
        {original.max.x, original.max.y, original.min.z},
        original.max,
    };

    glm::vec3 min = {INFINITY, INFINITY, INFINITY};
    glm::vec3 max = {-INFINITY, -INFINITY, -INFINITY};

    // Transform all of the corners, and keep track of the greatest and least
    // values we see on each coordinate axis.
    for (int i = 0; i < 8; i++)
    {
        glm::vec3 transformed = glm::vec3(transformation * glm::vec4(corners[i], 1.0f));
        min = glm::min(min, transformed);
        max = glm::max(max, transformed);
    }

    aabb aabb;
    aabb.min = min;
    aabb.max = max;

    return aabb;
}

struct Line
{
    unsigned int VBO, VAO;
    std::vector<float> vertices;

    Line(glm::vec3 start, glm::vec3 end)
    {
        vertices = {
            start.x,
            start.y,
            start.z,
            end.x,
            end.y,
            end.z,
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, 24, vertices.data(), GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void update(glm::vec3 const &start, glm::vec3 const &end)
    {
        vertices = {
            start.x,
            start.y,
            start.z,
            end.x,
            end.y,
            end.z,
        };

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, 24, vertices.data());
    }

    void draw()
    {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, 2);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    ~Line()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
};

struct Box
{
    unsigned int VBO, VAO;
    std::vector<float> vertices;

    Box(aabb b)
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

            // Loop 2: XY Z (max)
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

            // Lists:
            // 1
            b.min.x,
            b.min.y,
            b.min.z,
            b.min.x,
            b.min.y,
            b.max.z,
            // 2
            b.max.x,
            b.min.y,
            b.min.z,
            b.max.x,
            b.min.y,
            b.max.z,
            // 3
            b.max.x,
            b.max.y,
            b.min.z,
            b.max.x,
            b.max.y,
            b.max.z,
            // 4
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

    void update(aabb const &b)
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

            // Loop 2: XY Z (max)
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

            // Lists:
            // 1
            b.min.x,
            b.min.y,
            b.min.z,
            b.min.x,
            b.min.y,
            b.max.z,
            // 2
            b.max.x,
            b.min.y,
            b.min.z,
            b.max.x,
            b.min.y,
            b.max.z,
            // 3
            b.max.x,
            b.max.y,
            b.min.z,
            b.max.x,
            b.max.y,
            b.max.z,
            // 4
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

    void draw()
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

    ~Box()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
};

bool intersect(ray r, aabb b)
{
    // r.dir is unit direction vector of ray
    glm::vec3 dirfrac;
    dirfrac.x = 1.0f / r.dir.x;
    dirfrac.y = 1.0f / r.dir.y;
    dirfrac.z = 1.0f / r.dir.z;
    // lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
    // r.org is origin of ray
    float t1 = (b.min.x - r.org.x) * dirfrac.x;
    float t2 = (b.max.x - r.org.x) * dirfrac.x;
    float t3 = (b.min.y - r.org.y) * dirfrac.y;
    float t4 = (b.max.y - r.org.y) * dirfrac.y;
    float t5 = (b.min.z - r.org.z) * dirfrac.z;
    float t6 = (b.max.z - r.org.z) * dirfrac.z;

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

glm::vec3 cast_ray(double xpos, double ypos, glm::mat4 const &view, glm::mat4 const &projection)
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

    x_axis_arrow->box = GetTransformedBounds(x_axis_arrow->box_copy, x_axis_arrow->matrix);
    y_axis_arrow->box = GetTransformedBounds(y_axis_arrow->box_copy, y_axis_arrow->matrix);
    z_axis_arrow->box = GetTransformedBounds(z_axis_arrow->box_copy, z_axis_arrow->matrix);

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

    static ray r;

    // static Box box(models[0]->box);
    // box.update(models[0]->box);
    // box.draw();

    // static Box x_arrow(arrows[0]->box);
    // x_arrow.update(arrows[0]->box);
    // x_arrow.draw();

    // static Box y_arrow(arrows[1]->box);
    // y_arrow.update(arrows[1]->box);
    // y_arrow.draw();

    // static Box z_arrow(arrows[2]->box);
    // z_arrow.update(arrows[2]->box);
    // z_arrow.draw();

    auto &x = arrows[0];
    auto &y = arrows[1];
    auto &z = arrows[2];
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && !ImGui::IsAnyItemActive())
    {
        ImVec2 xy = ImGui::GetMousePos();
        glm::vec3 mouse_ray = cast_ray(xy.x, xy.y, view, projection);
        float t = 1000.0f;
        glm::vec3 mouse_ray_world = position + t * mouse_ray;
        r.org = line_start = position;
        r.dir = line_end = mouse_ray_world;
        mouse_ray_line.update(line_start, line_end);
        if (selected_model && !(x->drag || y->drag || z->drag))
        {
            for (const auto &arrow : arrows)
                arrow->drag = intersect(r, arrow->box);
        }
        if (!(x->drag || y->drag || z->drag))
        {
            selected_model = intersect(r, models[0]->box) ? models[0] : NULL;
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
                selected_model->box = GetTransformedBounds(selected_model->box_copy, selected_model->matrix);
                for (auto &arrow : arrows)
                {
                    arrow->matrix[3].x += move_x;
                    arrow->matrix[3].y += move_y;
                    arrow->matrix[3].z += move_z;
                    arrow->box = GetTransformedBounds(arrow->box_copy, arrow->matrix);
                }
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

    ImGui::SliderFloat("Pos X", &position.x, -4.0f, 4.0f);
    ImGui::SliderFloat("Pos Y", &position.y, -4.0f, 4.0f);
    ImGui::SliderFloat("Pos Z", &position.z, -4.0f, 4.0f);
    ImGui::SliderFloat("H", &horizontal_angle, -4.0f, 4.0f);
    ImGui::SliderFloat("V", &vertical_angle, -4.0f, 4.0f);

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
