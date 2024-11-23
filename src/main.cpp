#include <SDL.h>
#include "glad/glad.h"
#include "glm.hpp"

#include "shader.h"
#include "map.h"
#include "input.h"
#include "camera.h"

#define VIDEO_WIDTH 1920
#define VIDEO_HEIGHT 1080

static SDL_Window *window;
static SDL_GLContext context;
static camera cam;

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    window = SDL_CreateWindow(
        "borepack",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        VIDEO_WIDTH, VIDEO_HEIGHT,
        SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

    context = SDL_GL_CreateContext(window);
    gladLoadGL();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_SetWindowMouseGrab(window, SDL_TRUE);
    SDL_ShowWindow(window);
    SDL_GL_SetSwapInterval(0);

    // init shaders
    loadShader("shaders/surface.glsl", "SurfaceShader");
    loadShader("shaders/sky.glsl", "SkyShader");
    loadShader("shaders/water.glsl", "WaterShader");

    input in = {};

    cam = createCamera(90.0f, 1.78f, 4.0f, 4096.0f);
    cam.speed = 320.0f;
    cam.pos.x = 535.0f;
    cam.pos.y = 86.0f;
    cam.pos.z = -256.0f;
    cam.rotation.y = 180.0f;

    loadMap(argv[1]);

    uint64_t old_time = SDL_GetPerformanceCounter();
    float time = 0.0f;

    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

    int running = 1;
    while (running) {
        SDL_Event event;

        in.mouseXRel = 0;
        in.mouseYRel = 0;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
                break;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    running = false;
                    break;
                }
                in.keyboard[event.key.keysym.scancode] = 1;
            } else if (event.type == SDL_KEYUP) {
                in.keyboard[event.key.keysym.scancode] = 0;
            } else if (event.type == SDL_MOUSEMOTION) {
                in.mouseX = event.motion.x;
                in.mouseY = event.motion.y;
                in.mouseXRel = event.motion.xrel;
                in.mouseYRel = event.motion.yrel;
            }
        }

        uint64_t elapsed = SDL_GetPerformanceCounter() - old_time;
        float delta_time = (float)elapsed / SDL_GetPerformanceFrequency();
        old_time = SDL_GetPerformanceCounter();
        time += delta_time;

        cameraHandleUserInput(&cam, &in, delta_time);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawMap(time, &cam);

        SDL_GL_SwapWindow(window);
    }

    SDL_DestroyWindow(window);
    SDL_GL_DeleteContext(context);
    SDL_Quit();
    return 0;
}
