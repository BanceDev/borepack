#ifdef _WIN32
#include "SDL.h"
#else
#include <SDL2/SDL.h>
#endif
#include "glad/glad.h"
#include "map.h"
#include "world.h"
#include "camera.h"
#include <iostream>

enum Parameters
{
	WINDOW = 1,
	FULLWINDOW = 2,
	FULLSCREEN = 4,
	SHOWCURSOR = 8
};

#ifdef _WIN32
int WinMain()
#else
int main()
#endif
{
	Map map;
	World world;
	Camera camera;


	// Load map
	if (!map.Initialize("assets/start.bsp", "assets/palette.lmp")) {
		std::cerr << "[ERROR] main() Unable to initialize map" << std::endl;
		return -1;
	}

	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cerr << "[ERROR] main() Unable to initialize SDL" << std::endl;
		return -1;
	}

	// Get current display mode
	SDL_DisplayMode dm;
	if (SDL_GetCurrentDisplayMode(0, &dm) != 0) {
		std::cerr << "[ERROR] main() Unable to get current display mode" << std::endl;
		return -1;
	}

	// Cursor and mouse mode
	SDL_ShowCursor(0);

	// Set relative mouse mode
	if (SDL_SetRelativeMouseMode(SDL_TRUE) != 0) {
		std::cerr << "[ERROR] main() Unable to set mouse mode" << std::endl;
		return -1;
	}

	// Create window
	Uint32 flags = SDL_WINDOW_OPENGL;
	flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	SDL_Window *window = SDL_CreateWindow("Quake!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, dm.w, dm.h, flags);
	if (!window) {
		std::cerr << "[ERROR] main() Unable to create window" << std::endl;
		return -1;
	}

	// Create our opengl context and attach it to our window
	SDL_GLContext context = SDL_GL_CreateContext(window);
	if (!context) {
		std::cerr << "[ERROR] main() Unable to create context" << std::endl;
		return -1;
	}

	// Set OpenGL attributes
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetSwapInterval(1);

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		std::cerr << "[ERROR] Failed to initialize GLAD" << std::endl;
		return -1;
	}
	// Initialize world
	if (!world.Initialize(&map, dm.w, dm.h)) {
		std::cerr << "[ERROR] main() Unable to initialize world" << std::endl;
		return -1;
	}

	bool loop = true;
	while (loop) {
		SDL_PumpEvents();

		const Uint8 * keys = SDL_GetKeyboardState(NULL);

		// Handle keyboard events
		if (SDL_QuitRequested()) {
			loop = false;
			break;
		}
		if (keys[SDL_GetScancodeFromKey(SDLK_ESCAPE)]) {
			loop = false;
			break;
		}
		if (keys[SDL_GetScancodeFromKey(SDLK_w)]) {
			camera.MoveForward();
		}
		if (keys[SDL_GetScancodeFromKey(SDLK_UP)]) {
			camera.MoveForward();
		}
		if (keys[SDL_GetScancodeFromKey(SDLK_s)]) {
			camera.MoveBackward();
		}
		if (keys[SDL_GetScancodeFromKey(SDLK_DOWN)]) {
			camera.MoveBackward();
		}
		if (keys[SDL_GetScancodeFromKey(SDLK_RIGHT)]) {
			camera.TurnRight();
		}
		if (keys[SDL_GetScancodeFromKey(SDLK_LEFT)]) {
			camera.TurnLeft();
		}
		if (keys[SDL_GetScancodeFromKey(SDLK_d)]) {
			camera.StrafeRight();
		}
		if (keys[SDL_GetScancodeFromKey(SDLK_a)]) {
			camera.StrafeLeft();
		}
		if (keys[SDL_GetScancodeFromKey(SDLK_PAGEUP)]) {
			camera.PitchUp();
		}
		if (keys[SDL_GetScancodeFromKey(SDLK_PAGEDOWN)]) {
			camera.PitchDown();
		}

		// Handle mouse events
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_MOUSEMOTION) {
				camera.Yaw(event.motion.xrel);
				camera.Pitch(event.motion.yrel);
			}
		}

		// Update camera position
		camera.UpdatePosition();

		// Draw the scene
		world.DrawScene(&camera);
		SDL_GL_SwapWindow(window);
	}

	// Delete our OpengL context
	SDL_GL_DeleteContext(context);

	// Destroy our window
	SDL_DestroyWindow(window);

	// Shutdown SDL
	SDL_Quit();
}
