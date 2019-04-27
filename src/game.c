#include "game.h"

void reset_level(struct level *level) {
	level->width      = 0;
	level->height     = 0;
	level->layers     = 0;
	level->num_blocks = 0;
}

void build_level_from_strings(struct level *level, char ***strings) {
	for (u32 k = 0; k < level->layers; ++k) {
		for (u32 j = 0; j < level->height; ++j) {
			for (u32 i = 0; i < level->width; ++i) {
			}
		}
	}
}
