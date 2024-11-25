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
}

void Player::handleInput(input *in, float dt) {
    glm::vec3 wishDir(0.0f);

    // Forward/backward
    if (in->keyboard[SDL_SCANCODE_W]) wishDir.x += 1.0f;
    if (in->keyboard[SDL_SCANCODE_S]) wishDir.x -= 1.0f;

    // Strafe left/right
    if (in->keyboard[SDL_SCANCODE_D]) wishDir.y += 1.0f;
    if (in->keyboard[SDL_SCANCODE_A]) wishDir.y -= 1.0f;

    // Jump
    if (in->keyboard[SDL_SCANCODE_SPACE] && onGround) {
        vel.z = JUMP_FORCE;
        onGround = false;
    }

    // Normalize wish direction
    if (glm::length(wishDir) > 0.0f) {
        wishDir = glm::normalize(wishDir);
    }

    // Apply movement using slide move
    glm::vec3 finalMove = slideMove(wishDir, dt);
    pos += finalMove;

    // Update camera position to match player position
    cam.pos = pos + glm::vec3(0, 0, bbox.max.z - 8); // Eye position slightly below top of bbox
}

void Player::update(float dt) {
    applyFriction(dt);
    applyGravity(dt);

    glm::vec3 newPos = pos + vel * dt;
    if (checkCollision(newPos)) {
        slideMove(glm::normalize(vel), dt);
    } else {
        pos = newPos;
    }

    glm::vec3 groundCheck = pos;
    groundCheck.z -=1.0f;
    onGround = checkCollision(groundCheck);
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
        vel.z -= GRAVITY * dt;
    }
}

glm::vec3 Player::slideMove(const glm::vec3 &wishDir, float dt) {
    glm::mat4 rotMatrix4 = glm::yawPitchRoll(cam.rotation.y, cam.rotation.x, cam.rotation.z);
    glm::mat3 rotMatrix = glm::mat3(rotMatrix4);

    glm::vec3 worldWishDir = rotMatrix * wishDir;
    glm::vec3 moveAmount = worldWishDir * (float)MOVE_SPEED * dt;

    if (checkCollision(pos + moveAmount)) {
        glm::vec3 normal = glm::vec3(0.0f);
        float d = glm::dot(moveAmount, normal);
        glm::vec3 slide = moveAmount - normal * d;

        if (!checkCollision(pos + slide)) {
            return slide;
        }
        return glm::vec3(0.0f);
    }

    return moveAmount;
}
