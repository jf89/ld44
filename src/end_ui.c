#include "end_ui.h"

#include "gl_3_3.h"
#include "opengl.h"

void run_end_ui(SDL_Window *window) {
	f32 start_time = ((f32)SDL_GetTicks()) / 1000.0f;
	f32 end_time = start_time + 3.0f;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	while (1) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				return;
			case SDL_KEYUP:
				switch (e.key.keysym.sym) {
				case SDLK_q:
					return;
				}
				break;
			}
		}

		f32 time = ((f32)SDL_GetTicks()) / 1000.0f;
		if (time > end_time) {
			break;
		}
		f32 fade = 0.0f;
		f32 fr = 0.0f, fg = 0.0f, fb = 0.0f;
		if (time - start_time < 0.8f) {
			fade = 1.0f - (time - start_time) / 0.8f;
		} else if ((time - start_time) > 2.0f) {
			fr = fg = fb = 1.0f;
			fade = ((time - start_time) - 2.0f) / 1.0f;
		}
		set_fade_color(fr, fg, fb, fade);

		glClear(GL_COLOR_BUFFER_BIT);
		reset_characters();
		add_string("Thank you",
			(struct color) { .r = 0.75f, .g = 0.75f, .b = 0.75f },
			6.0f, 4.0f, -3.0f);
		add_string("  for playing.",
			(struct color) { .r = 0.75f, .g = 0.75f, .b = 0.75f },
			6.0f, 4.0f, -4.0f);
		draw_characters();
		draw_fade();
		SDL_GL_SwapWindow(window);
	}
}
