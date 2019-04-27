#pragma once

#include <SDL.h>

#include "game.h"

enum outcome {
	OUTCOME_DEATH,
	OUTCOME_SUCCESS,
	OUTCOME_QUIT,
};

enum outcome run_game_ui(SDL_Window *window, struct level *level);
