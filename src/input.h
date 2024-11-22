#pragma once
#include <SDL.h>

struct input {
    int mouseX;
    int mouseY;
    int mouseXRel;
    int mouseYRel;
    int mouseButton;
    int keyboard[SDL_NUM_SCANCODES];
};

