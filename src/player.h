#pragma once

#include <SDL.h>
#include "camera.h"
#include "glm.hpp"
#include "renderer.h"
#include "map.h"

#define JUMP_FORCE 270.0f
#define MAX_SPEED 320.0f
#define MAX_ACCEL 3200.0f
#define GRAVITY 800.0f
#define FRICTION 6.0f

struct input {
    int mouseX;
    int mouseY;
    int mouseXRel;
    int mouseYRel;
    int mouseButton;
    int keyboard[SDL_NUM_SCANCODES];
};

class Player {
public:
    Player();
    void spawn();
    glm::vec3 handleInput(input *in, float dt);
    void update(input *in, float dt);

    Camera cam;

    // physics properties
    glm::vec3 vel;
    glm::vec3 pos;
    bool onGround;
    float offGroundTimer;
    aabb bbox;
private:
    void applyFriction(float dt);
    void applyGravity(float dt);
    void checkGroundStatus(float dt);
    bool checkCollision(const glm::vec3 &newPos, glm::vec3 *normal);
    bool checkBSPCollision(int nodeIndex, const glm::vec3 &newPos, const glm::vec3 &maxs, glm::vec3 *normal);
    float classifyBox(const bsp_plane* plane, const glm::vec3& point);
    glm::vec3 slideMove(const glm::vec3 &wishDir, float dt);
    glm::vec3 computeSlide(const glm::vec3& move, const glm::vec3& normal);
};
