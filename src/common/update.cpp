#include <vector>

#include "impl_base.hpp"
#include "shader.hpp"
#include "sphere.hpp"
#include "update.hpp"

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
std::vector<Sphere> spheres;

void graph_ops_init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glDepthFunc(GL_LESS);

    program_id = load_shaders();
    matrix_id = glGetUniformLocation(program_id, "MVP");
    time_id = glGetUniformLocation(program_id, "u_time");
    color_id = glGetUniformLocation(program_id, "u_color");

    spheres = {
        {matrix_id, color_id, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), 1.0f, 64, 32},
    };
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

    for (const auto &sphere : spheres)
        sphere.draw(view_projection);
}

void imgui_update()
{
    ImGui::Begin("graph-ops");

    if (ImGui::Button("Create Sphere"))
        spheres.push_back(Sphere(matrix_id, color_id, glm::vec4(1.0f, 1.0f, 0.0f, 0.7f), 1.0f, 64, 32));

    for (size_t i = 0; i < spheres.size(); ++i)
    {
        ImGui::PushID(i);
        ImGui::Text("Sphere %zu", i + 1);
        ImGui::SliderFloat("X", &spheres[i].model[3][0], -4.0f, 4.0f);
        ImGui::SliderFloat("Y", &spheres[i].model[3][1], -4.0f, 4.0f);
        ImGui::SliderFloat("Z", &spheres[i].model[3][2], -4.0f, 4.0f);
        ImGui::ColorPicker4("Color", (float *)&spheres[i].color);
        ImGui::PopID();
    }

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();
}
