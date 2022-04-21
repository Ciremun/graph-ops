#pragma once

#include <glm/glm.hpp>

#include "model.hpp"

extern float horizontal_angle;
extern float vertical_angle;
extern float speed;
extern float mouse_speed;
extern glm::highp_mat4 projection;
extern std::vector<Model *> arrows;
extern Model *selected_model;

void graph_ops_init();
void graph_ops_update(double ticks, double dt);
void imgui_update();
