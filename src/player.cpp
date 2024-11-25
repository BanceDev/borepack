#define GLM_ENABLE_EXPERIMENTAL
#include "player.h"
#include "geometric.hpp"
#include "map.h"
#include "gtx/euler_angles.hpp"
#include <iostream>
#include <sstream>

Player::Player() {
    bbox.min = glm::vec3(-16, -16, -32);
    bbox.max = glm::vec3(16, 16, 32);

    vel = glm::vec3(0.0f);
    pos = glm::vec3(0.0f);
    onGround = false;
}

void Player::spawn() {
    std::string ents = getEntities();
    size_t info_pos = ents.find("\"info_player_start\"");
    // find the origin of the player start entity
    size_t origin_pos = ents.find("\"origin\"", info_pos);
    size_t start_quote = ents.find("\"", origin_pos + 8); // skip "origin"
    size_t end_quote = ents.find("\"", start_quote + 1);

    std::string origin_values = ents.substr(start_quote + 1, end_quote - start_quote - 1);

    std::istringstream iss(origin_values);
    int quake_x, quake_y, quake_z;
    if (!(iss >> quake_x >> quake_y >> quake_z)) {
        std::cerr << "failed to parse spawn point" << std::endl;
        return;
    }
    // Set player position (feet position)
    pos.x = quake_x;        // X stays the same
    pos.y = quake_z;        // Quake Z becomes Y in OpenGL
    pos.z = -quake_y;       // Negative Quake Y becomes Z in OpenGL

    // Set camera position (eye position = player position + eye height)
    cam.pos = pos;
    cam.pos.y += 22;  // Eye height offset

    // Reset physics state
    vel = glm::vec3(0.0f);
    onGround = false;
    cam.rotation = glm::vec3(0.0f, 180.0f, 0.0f);
}

void Player::handleInput(input *in, float dt) {
    float speed = cam.speed;
    float sens = 70.0f;
    if (in->keyboard[SDL_SCANCODE_LSHIFT]) speed *= 2.0f;
    if (in->keyboard[SDL_SCANCODE_W]) cam.pos += cam.getForwardVector(cam.rotation) * speed * dt;
    if (in->keyboard[SDL_SCANCODE_S]) cam.pos -= cam.getForwardVector(cam.rotation) * speed * dt;
    if (in->keyboard[SDL_SCANCODE_A]) cam.pos += cam.getRightVector(cam.rotation) * speed * dt;
    if (in->keyboard[SDL_SCANCODE_D]) cam.pos -= cam.getRightVector(cam.rotation) * speed * dt;
    if (in->keyboard[SDL_SCANCODE_E]) cam.pos += global_up_vec * speed * dt;
    if (in->keyboard[SDL_SCANCODE_Q]) cam.pos -= global_up_vec * speed * dt;
    cam.rotation.x += (float)in->mouseYRel * sens * dt;
    cam.rotation.y -= (float)in->mouseXRel * sens * dt;

    cam.rotation.x = glm::clamp(cam.rotation.x, -89.0f, 89.0f);
}

void Player::update(float dt) {
}

void Player::applyFriction(float dt) {
    if (!onGround) return;

    float speed = glm::length(vel);
    if (speed < 1.0f) {
        vel.x = vel.y = 0.0f;
        return;
    }

    float drop = speed * FRICTION * dt;
    float newSpeed = std::max(0.0f, speed - drop);
    vel *= (newSpeed / speed);
}

void Player::applyGravity(float dt) {
    if (!onGround) {
        //vel.y -= GRAVITY * dt;
    }
}

glm::vec3 Player::slideMove(const glm::vec3 &wishDir, float dt) {
    return glm::vec3(0.0f);
}

bool Player::checkCollision(const glm::vec3 &newPos) {
    return false;
}
