#pragma once
#include "glm.hpp"

static const glm::vec3 global_right_vec   = glm::vec3(1.0f, 0.0f, 0.0f);
static const glm::vec3 global_up_vec      = glm::vec3(0.0f, 1.0f, 0.0f);
static const glm::vec3 global_forward_vec = glm::vec3(0.0f, 0.0f, 1.0f);

class Camera {
public:
    Camera();
    glm::mat4 getViewMatrix();

    float speed;
    float fov;
    float aspect;
    float near;
    float far;
    glm::vec3 pos;
    glm::vec3 rotation;
    glm::mat4 projection_mtx;
    glm::vec3 getForwardVector(glm::vec3 rotation);
    glm::vec3 getRightVector(glm::vec3 rotation);

};
