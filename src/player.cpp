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
    pos.y = quake_z + 2.0f;        // Quake Z becomes Y in OpenGL
    pos.z = -quake_y;       // Negative Quake Y becomes Z in OpenGL

    // Set camera position (eye position = player position + eye height)
    cam.pos = pos;
    cam.pos.y += 22;  // Eye height offset

    // Reset physics state
    vel = glm::vec3(0.0f);
    onGround = false;
    cam.rotation = glm::vec3(0.0f, 180.0f, 0.0f);
}

glm::vec3 Player::handleInput(input *in, float dt) {
    float sens = 70.0f;
    // Clamp pitch to prevent camera from flipping
    cam.rotation.x = glm::clamp(cam.rotation.x + (float)in->mouseYRel * sens * dt, -89.0f, 89.0f);
    cam.rotation.y -= (float)in->mouseXRel * sens * dt;

    // Create view matrix for correct movement
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    viewMatrix = glm::rotate(viewMatrix, glm::radians(cam.rotation.y), glm::vec3(0, 1, 0)); // Yaw
    viewMatrix = glm::rotate(viewMatrix, glm::radians(cam.rotation.x), glm::vec3(1, 0, 0)); // Pitch

    // Get forward and right vectors from view matrix
    glm::vec3 forwardVector = glm::vec3(viewMatrix[2]);
    glm::vec3 rightVector = glm::vec3(viewMatrix[0]);

    // Flatten vectors to horizontal plane for movement
    forwardVector.y = 0;
    rightVector.y = 0;
    forwardVector = glm::normalize(forwardVector);
    rightVector = glm::normalize(rightVector);

    glm::vec3 wishDir(0, 0, 0);

    // Forward/backward
    if (in->keyboard[SDL_SCANCODE_W]) wishDir += forwardVector;
    if (in->keyboard[SDL_SCANCODE_S]) wishDir -= forwardVector;

    // Strafe left/right
    if (in->keyboard[SDL_SCANCODE_D]) wishDir -= rightVector;
    if (in->keyboard[SDL_SCANCODE_A]) wishDir += rightVector;

    // Normalize wish direction
    if (glm::length(wishDir) > 0.0f) {
        wishDir = glm::normalize(wishDir);
    }

    // Jump
    if (in->keyboard[SDL_SCANCODE_SPACE] && onGround) {
        vel.y = JUMP_FORCE;
        onGround = false;
    }

    return wishDir;
}


void Player::update(input *in, float dt) {
    // update physics state
    glm::vec3 wishDir = handleInput(in, dt);
    applyFriction(dt);

    float currentSpeed = glm::dot(vel, wishDir);
    float add_speed = glm::clamp(MAX_SPEED - currentSpeed, 0.0f, MAX_ACCEL * dt);
    glm::vec3 hVel = vel;
    hVel.y = 0;
    hVel += add_speed * wishDir;

    // gravity
    applyGravity(dt);
    checkGroundStatus(dt);

    vel.x = vel.z = 0;
    vel += hVel;
    pos += slideMove(vel, dt);

    // Update camera position to match player position
    cam.pos = pos + glm::vec3(0, 22, 0); // Eye position slightly below top of bbox
}

void Player::checkGroundStatus(float dt) {
    // Slightly extended ground check distance
    float groundCheckDistance = 1.5f;

    // Check if any of the points detect ground
    onGround = false;
    glm::vec3 groundCheck = pos;
    groundCheck.y -= groundCheckDistance;

    if (checkCollision(groundCheck, nullptr)) {
        onGround = true;

        // Optional: Snap to ground if very close
        if (std::abs(pos.y - groundCheck.y) < groundCheckDistance * 0.5f) {
            pos.y = groundCheck.y + bbox.min.y;
            vel.y = 0.0f;
        }

    }
}

void Player::applyGravity(float dt) {
    // Only apply gravity if not on ground
    if (!onGround) {
        // Consider using a more controlled gravity
        vel.y -= GRAVITY * dt;

        // Optional: Add terminal velocity to prevent excessive speed
        const float TERMINAL_VELOCITY = -2000.0f;
        vel.y = std::max(vel.y, TERMINAL_VELOCITY);
    }
}

void Player::applyFriction(float dt) {
    if (!onGround) return;

    float speed = glm::length(vel);
    if (speed < 1.0f) {
        vel.x = vel.z = 0.0f;
        return;
    }

    float drop = speed * FRICTION * dt;
    float newSpeed = std::max(0.0f, speed - drop);
    vel *= (newSpeed / speed);
}

glm::vec3 Player::slideMove(const glm::vec3& wishDir, float dt) {
    // Calculate movement
    glm::vec3 moveAmount = wishDir * dt;
    glm::vec3 normal;

    // Check for collision
    if (checkCollision(pos + moveAmount, &normal)) {
        // Project movement onto the collision plane
        float d = glm::dot(moveAmount, normal);
        glm::vec3 slide = moveAmount - normal * d;

        // Check if sliding movement is valid
        if (!checkCollision(pos + slide, nullptr)) {
            return slide;
        }
        return glm::vec3(0.0f);
    }

    return moveAmount;
}

bool Player::checkCollision(const glm::vec3& newPos, glm::vec3 *normal) {
    glm::vec3 quakePos = glm::vec3(
        newPos.x,
        -newPos.z,
        newPos.y
    );

    bsp_model worldModel = loaded_map.models[0];
    int headNode = worldModel.head_nodes[0];

    glm::vec3 mins = quakePos + bbox.min;
    glm::vec3 maxs = quakePos + bbox.max;

    return checkBSPCollision(headNode, mins, maxs, normal);
}

bool Player::checkBSPCollision(int nodeIndex, const glm::vec3 &mins, const glm::vec3 &maxs, glm::vec3 *normal) {
    // Check if we've hit a leaf
    if (nodeIndex < 0) {
        bsp_leaf* leaf = &loaded_map.leafs[~nodeIndex];
        // If it's solid, we have a collision
        return (leaf->contents == BSP_CONTENTS_SOLID);
    }

    bsp_node* node = &loaded_map.nodes[nodeIndex];
    bsp_plane* plane = &loaded_map.planes[node->plane];
    if (normal) {
        *normal = plane->normal;
    }
    // Classify box against plane
    float d1 = classifyBox(plane, mins);
    float d2 = classifyBox(plane, maxs);

    // Check children based on classification
    if (d1 >= 0 && d2 >= 0)
        return checkBSPCollision(node->children[0], mins, maxs, normal);
    if (d1 < 0 && d2 < 0)
        return checkBSPCollision(node->children[1], mins, maxs, normal);

    // Box spans the plane, must check both sides
    return checkBSPCollision(node->children[0], mins, maxs, normal) ||
           checkBSPCollision(node->children[1], mins, maxs, normal);
}

float Player::classifyBox(const bsp_plane* plane, const glm::vec3& point) {
    switch (plane->type) {
        case PLANE_X:
            return point.x - plane->dist;
        case PLANE_Y:
            return point.y - plane->dist;
        case PLANE_Z:
            return point.z - plane->dist;
        default:
            return glm::dot(point, plane->normal) - plane->dist;
    }
}
