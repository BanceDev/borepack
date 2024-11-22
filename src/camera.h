#pragma once
#include "glm.hpp"
#include "input.h"

struct camera {
    float speed;
    float fov;
    float aspect;
    float near;
    float far;
    glm::vec3 pos;
    glm::vec3 rotation;
    glm::mat4 projection_mtx;
};


glm::vec3 getForwardVector(glm::vec3 rotation);
glm::vec3 getRightVector(glm::vec3 rotation);
camera createCamera(float Fov, float Aspect, float Near, float Far);
glm::mat4 cameraGetViewMatrix(camera *camera);
void cameraHandleUserInput(camera *camera, input *in, float dt);
