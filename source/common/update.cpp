#include <algorithm>
#include <cstdio>
#include <iterator>
#include <string>
#include <vector>

#include "impl_base.hpp"
#include "shader.hpp"
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
// std::vector<Sphere*> spheres;
// std::vector<Sphere*> spheres_imgui_draw_order;
// std::vector<Sphere*>::iterator spheres_mid_it;

std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;
std::vector<glm::vec3> normals; // Won't be used at the moment.

GLuint vertexbuffer;

bool loadOBJ(
    const char *path,
    std::vector<glm::vec3> &out_vertices,
    std::vector<glm::vec2> &out_uvs,
    std::vector<glm::vec3> &out_normals)
{
    printf("Loading OBJ file %s...\n", path);

    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
        getchar();
        return false;
    }

    while (1)
    {

        char lineHeader[128];
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.

        // else : parse lineHeader

        if (strcmp(lineHeader, "v") == 0)
        {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        }
        else if (strcmp(lineHeader, "vt") == 0)
        {
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
            uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
            temp_uvs.push_back(uv);
        }
        else if (strcmp(lineHeader, "vn") == 0)
        {
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0)
        {
            std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%u/%u/%u %u/%u/%u %u/%u/%u\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
            if (matches != 9)
            {
                printf("File can't be read by our simple parser :-( Try exporting with other options\n");
                fclose(file);
                return false;
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices.push_back(uvIndex[0]);
            uvIndices.push_back(uvIndex[1]);
            uvIndices.push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }
        else
        {
            // Probably a comment, eat up the rest of the line
            char stupidBuffer[1000];
            fgets(stupidBuffer, 1000, file);
        }
    }

    // For each vertex of each triangle
    for (unsigned int i = 0; i < vertexIndices.size(); i++)
    {

        // Get the indices of its attributes
        unsigned int vertexIndex = vertexIndices[i];
        unsigned int uvIndex = uvIndices[i];
        unsigned int normalIndex = normalIndices[i];

        // Get the attributes thanks to the index
        glm::vec3 vertex = temp_vertices[vertexIndex - 1];
        glm::vec2 uv = temp_uvs[uvIndex - 1];
        glm::vec3 normal = temp_normals[normalIndex - 1];

        // Put the attributes in buffers
        out_vertices.push_back(vertex);
        out_uvs.push_back(uv);
        out_normals.push_back(normal);
    }
    fclose(file);
    return true;
}

void sort_spheres()
{
    // spheres_mid_it = std::partition(spheres.begin(), spheres.end(), [](Sphere *s){ return s->color.a == 1.0f; });
}

void graph_ops_init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

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

    sort_spheres();

    // spheres_imgui_draw_order = spheres;

    loadOBJ("models/test.obj", vertices, uvs, normals);

    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
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

    auto mvp = view_projection * glm::mat4(1.0f);
    glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvp[0][0]);
    glUniform4f(color_id, 0.0f, 1.0f, 0.0f, .7f);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        0,        // attribute
        3,        // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0,        // stride
        (void *)0 // array buffer offset
    );

    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    glDisableVertexAttribArray(0);
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
