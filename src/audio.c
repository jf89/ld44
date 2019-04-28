#include "audio.h"

#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <SDL_mixer.h>

static Mix_Chunk *mix_chunk_move    = NULL;
static Mix_Chunk *mix_chunk_heart   = NULL;
static Mix_Chunk *mix_chunk_fall    = NULL;
static Mix_Chunk *mix_chunk_hurt    = NULL;
static Mix_Chunk *mix_chunk_victory = NULL;

static Mix_Chunk *load_sound(char *filename) {
	char *base_dir = SDL_GetBasePath();
	u32 len = strlen(filename) + strlen(base_dir) + 1;
	char *full_filename = malloc(len * sizeof(char));
	full_filename[0] = '\0';
	strcat(full_filename, base_dir);
	strcat(full_filename, filename);
	SDL_free(base_dir);
	Mix_Chunk *result = Mix_LoadWAV(full_filename);
	free(full_filename);
	return result;
}

i32 init_audio(void) {
	mix_chunk_move = load_sound("move.wav");
	if (mix_chunk_move == NULL) {
		SDL_Log("Error loading 'move.wav': %s", Mix_GetError());
		goto error_load_move;
	}
	mix_chunk_heart = load_sound("heart.wav");
	if (mix_chunk_heart == NULL) {
		SDL_Log("Error loading 'heart.wav': %s", Mix_GetError());
		goto error_load_heart;
	}
	mix_chunk_fall = load_sound("fall.wav");
	if (mix_chunk_fall == NULL) {
		SDL_Log("Error loading 'fall.wav': %s", Mix_GetError());
		goto error_load_fall;
	}
	mix_chunk_hurt = load_sound("hurt.wav");
	if (mix_chunk_hurt == NULL) {
		SDL_Log("Error loading 'hurt.wav': %s", Mix_GetError());
		goto error_load_hurt;
	}
	mix_chunk_victory = load_sound("victory.wav");
	if (mix_chunk_victory == NULL) {
		SDL_Log("Error loading 'victory.wav': %s", Mix_GetError());
		goto error_load_victory;
	}
	Mix_AllocateChannels(16);
	Mix_Volume(-1, MIX_MAX_VOLUME / 4);
	return 0;

error_load_victory:
	Mix_FreeChunk(mix_chunk_hurt);
error_load_hurt:
	Mix_FreeChunk(mix_chunk_fall);
error_load_fall:
	Mix_FreeChunk(mix_chunk_heart);
error_load_heart:
	Mix_FreeChunk(mix_chunk_move);
error_load_move:
	return 1;
}

void quit_audio(void) {
	Mix_FreeChunk(mix_chunk_move);
	Mix_FreeChunk(mix_chunk_heart);
	Mix_FreeChunk(mix_chunk_fall);
	Mix_FreeChunk(mix_chunk_hurt);
	Mix_FreeChunk(mix_chunk_victory);
}

void play_sound(enum sound sound) {
	switch (sound) {
	case SOUND_MOVE:
		Mix_PlayChannel(-1, mix_chunk_move, 0);
		break;
	case SOUND_HEART:
		Mix_PlayChannel(-1, mix_chunk_heart, 0);
		break;
	case SOUND_FALL:
		Mix_PlayChannel(-1, mix_chunk_fall, 0);
		break;
	case SOUND_HURT:
		Mix_PlayChannel(-1, mix_chunk_hurt, 0);
		break;
	case SOUND_VICTORY:
		Mix_PlayChannel(-1, mix_chunk_victory, 0);
		break;
	}
}
