#pragma once

#include <SDL.h>
#include "camera.h"
#include "glm.hpp"
#include "renderer.h"

#define JUMP_FORCE 270
#define MOVE_SPEED 200
#define GRAVITY 800
#define FRICTION 6

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
    void handleInput(input *in, float dt);
    void update(float dt);

    camera cam;

    // physics properties
    glm::vec3 vel;
    glm::vec3 pos;
    bool onGround;
    aabb bbox;
private:
    void applyFriction(float dt);
    void applyGravity(float dt);
    bool checkCollision(const glm::vec3 &newPos);
    bool checkBSPCollision(int nodeIndex, const glm::vec3 &mins, const glm::vec3 &maxs);
    glm::vec3 slideMove(const glm::vec3 &wishDir, float dt);
};
