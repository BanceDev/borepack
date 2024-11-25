#include "gtc/type_ptr.hpp"
#include "camera.h"

Camera::Camera() {
    fov = 90;
    aspect = 1.78f;
    near = 1.0f;
    far = 4096.0f;
    projection_mtx = glm::perspective(glm::radians(fov), aspect, near, far);
    speed = 320.0f;
}

glm::vec3 Camera::getForwardVector(glm::vec3 rotation) {
    glm::vec3 rads_rot = glm::radians(rotation);
    glm::vec3 Result = glm::quat(rads_rot) * global_forward_vec;
    return Result;
}

glm::vec3 Camera::getRightVector(glm::vec3 rotation) {
    glm::vec3 rads_rot = glm::radians(rotation);
    glm::vec3 Result = glm::quat(rads_rot) * global_right_vec;
    return Result;
}

glm::mat4 Camera::getViewMatrix() {
    glm::vec3 target = pos + getForwardVector(rotation);
    glm::mat4 result = glm::lookAt(pos, target, global_up_vec);
    return result;
}
