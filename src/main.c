#include <stdlib.h>
#include <SDL.h>

#include "gl_3_3.h"
#include "opengl.h"

i32 main(i32 argc, char *argv[]) {
	i32 exit_success = EXIT_FAILURE;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
		SDL_Log("Unable to initialize SDL : %s", SDL_GetError());
		goto error_failed_init;
	};

	if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
			SDL_GL_CONTEXT_PROFILE_CORE) != 0) {
		SDL_Log("Failed to set GL context profile mask: %s",
			SDL_GetError());
		goto error_gl_context_profile;
	}
	if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) != 0) {
		SDL_Log("Failed to set GL context major version: %s",
			SDL_GetError());
		goto error_set_gl_major_version;
	}
	if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3) != 0) {
		SDL_Log("Failed to set GL context minor version: %s",
			SDL_GetError());
		goto error_set_gl_minor_version;
	}

	SDL_Window *window = SDL_CreateWindow("ld44",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
	if (window == NULL) {
		SDL_Log("Unable to create window: %s", SDL_GetError());
		goto error_failed_window;
	}

	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	if (gl_context == NULL) {
		SDL_Log("Unable to create GL context: %s", SDL_GetError());
		goto error_failed_gl_context;
	}

	SDL_GL_SetSwapInterval(1);

#define GL_FUNC(_return_value, name, ...) \
	name = SDL_GL_GetProcAddress(#name); \
	if (name == NULL) { \
		SDL_Log("Failed to load GL function '" #name "' %s", \
			SDL_GetError()); \
		goto error_failed_load_gl_function; \
	}
	GL_3_3_FUNCTIONS
#undef GL_FUNC

	if (init_opengl()) {
		goto error_failed_init_opengl;
	}

	// success
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	while (1) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				goto successful_exit;
			}
		}
		// glClear(GL_COLOR_BUFFER_BIT);
		test_draw();
		SDL_GL_SwapWindow(window);
	}

successful_exit:
	exit_success = EXIT_SUCCESS;

	quit_opengl();
error_failed_init_opengl:
error_failed_load_gl_function:
	SDL_GL_DeleteContext(gl_context);
error_failed_gl_context:
	SDL_DestroyWindow(window);
error_failed_window:
error_set_gl_minor_version:
error_set_gl_major_version:
error_gl_context_profile:
	SDL_Quit();
error_failed_init:
	return exit_success;
}