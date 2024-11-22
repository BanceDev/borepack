#include "gtc/type_ptr.hpp"
#include "camera.h"

static const glm::vec3 global_right_vec   = glm::vec3(1.0f, 0.0f, 0.0f);
static const glm::vec3 global_up_vec      = glm::vec3(0.0f, 1.0f, 0.0f);
static const glm::vec3 global_forward_vec = glm::vec3(0.0f, 0.0f, 1.0f);

glm::vec3 getForwardVector(glm::vec3 rotation) {
    glm::vec3 rads_rot = glm::radians(rotation);
    glm::vec3 Result = glm::quat(rads_rot) * global_forward_vec;
    return Result;
}

glm::vec3 getRightVector(glm::vec3 rotation) {
    glm::vec3 rads_rot = glm::radians(rotation);
    glm::vec3 Result = glm::quat(rads_rot) * global_right_vec;
    return Result;
}

camera createCamera(float fov, float aspect, float near, float far) {
    camera result = {};
    result.fov = fov;
    result.aspect = aspect;
    result.near = near;
    result.far = far;
    result.projection_mtx = glm::perspective(glm::radians(fov), aspect, near, far);
    return result;
}

glm::mat4 cameraGetViewMatrix(camera *camera) {
    glm::vec3 target = camera->pos + getForwardVector(camera->rotation);
    glm::mat4 result = glm::lookAt(camera->pos, target, global_up_vec);
    return result;
}

void cameraHandleUserInput(camera *camera, input *in, float dt) {
    float speed = camera->speed;
    float sens = 70.0f;
    if (in->keyboard[SDL_SCANCODE_LSHIFT]) speed *= 2.0f;
    if (in->keyboard[SDL_SCANCODE_W]) camera->pos += getForwardVector(camera->rotation) * speed * dt;
    if (in->keyboard[SDL_SCANCODE_S]) camera->pos -= getForwardVector(camera->rotation) * speed * dt;
    if (in->keyboard[SDL_SCANCODE_A]) camera->pos += getRightVector(camera->rotation) * speed * dt;
    if (in->keyboard[SDL_SCANCODE_D]) camera->pos -= getRightVector(camera->rotation) * speed * dt;
    if (in->keyboard[SDL_SCANCODE_E]) camera->pos += global_up_vec * speed * dt;
    if (in->keyboard[SDL_SCANCODE_Q]) camera->pos -= global_up_vec * speed * dt;
    camera->rotation.x += (float)in->mouseYRel * sens * dt;
    camera->rotation.y -= (float)in->mouseXRel * sens * dt;

    camera->rotation.x = glm::clamp(camera->rotation.x, -89.0f, 89.0f);
}
