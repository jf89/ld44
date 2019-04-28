#pragma once

enum sound {
	SOUND_MOVE,
	SOUND_HEART,
	SOUND_FALL,
	SOUND_HURT,
	SOUND_VICTORY,
};

i32 init_audio(void);
void quit_audio(void);

void play_sound(enum sound sound);
