#include <SDL.h>
#include "glad/glad.h"
#include "glm.hpp"
#include "gtc/type_ptr.hpp"

#include <stdio.h>
#include <vector>
#include <time.h>

#include "renderer.h"
#include "bsp.h"
#include "map.h"

#define VIDEO_WIDTH 1920
#define VIDEO_HEIGHT 1080

struct input {
    int32_t mouseX;
    int32_t mouseY;
    int32_t mouseXRel;
    int32_t mouseYRel;
    int32_t mouseButton;
    int32_t keyboard[SDL_NUM_SCANCODES];
};

static SDL_Window *window;
static SDL_GLContext context;
static camera cam;
