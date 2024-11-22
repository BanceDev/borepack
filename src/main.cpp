#include <SDL.h>
#include "glad/glad.h"
#include "glm.hpp"

#include <stdio.h>
#include <vector>
#include <time.h>

#include "renderer.h"
#include "bsp.h"
#include "map.h"
#include "input.h"

#define VIDEO_WIDTH 1920
#define VIDEO_HEIGHT 1080

static SDL_Window *window;
static SDL_GLContext context;
static camera cam;

int SDL_main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    window = SDL_CreateWindow(
        "Harm",
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

    // Init resources
    LoadShader("shaders/surface.glsl", "SurfaceShader");
    LoadShader("shaders/sky.glsl", "SkyShader");
    LoadShader("shaders/gui.glsl", "GuiShader");
    LoadShader("shaders/debug.glsl", "DebugShader");

    input Input = {};

    float AspectRatio = (float)VIDEO_WIDTH / VIDEO_HEIGHT;
    cam = createCamera(90.0f, AspectRatio, 0.1f, 65536.0f);
    cam.speed = 320.0f;
    cam.pos.x = 535.0f;
    cam.pos.y = 86.0f;
    cam.pos.z = -256.0f;
    cam.rotation.y = 180.0f;

    loadMap(argv[1]);

    uint32_t Indices[128] = {};
    u64 OldTime = SDL_GetPerformanceCounter();
    f32 Time = 0.0f;

    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    
    b32 Running = 1;
    while (Running)
    {
        SDL_Event event;

        Input.MousePositionXRel = 0;
        Input.MousePositionYRel = 0;
        
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                Running = false;
                break;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                {
                    Running = false;
                    break;
                }
                Input.Keyboard[event.key.keysym.scancode] = 1;
            }
            else if (event.type == SDL_KEYUP)
            {
                Input.Keyboard[event.key.keysym.scancode] = 0;

                if (event.key.keysym.scancode == SDL_SCANCODE_F5)
                {
                }
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                Input.MousePositionX = event.motion.x;
                Input.MousePositionY = event.motion.y;
                Input.MousePositionXRel = event.motion.xrel;
                Input.MousePositionYRel = event.motion.yrel;
            }
        }

        u64 Elapsed = SDL_GetPerformanceCounter() - OldTime;
        f32 DeltaTime = (f32)Elapsed / SDL_GetPerformanceFrequency();
        u32 FramesPerSecond = (s32)(1.0f / DeltaTime);
        OldTime = SDL_GetPerformanceCounter();
        Time += DeltaTime;

        CameraHandleUserInput(&Camera, &Input, DeltaTime);

        GuiBegin();

        GuiDrawText(8, 8,  "MS %3d", (s32)(DeltaTime * 1000.0f));
        GuiDrawText(8, 32, "FPS %d", (s32)(1.0f / DeltaTime));

        u64 FrameTime = SDL_GetPerformanceCounter();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
        DrawMap(Time);

        s32 FrameTimeDelta = SDL_GetPerformanceCounter() - FrameTime;
        f32 RenderTimeMS = ((f32)FrameTimeDelta / SDL_GetPerformanceFrequency()) * 1000.0f;

        GuiEnd();

        SDL_GL_SwapWindow(Window);
    }

    SDL_DestroyWindow(Window);
    SDL_GL_DeleteContext(Context);
    SDL_Quit();
    return 0;
}
