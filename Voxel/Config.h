#pragma once
#include "PerlinNoise.hpp"

namespace Config {
	constexpr int CHUNK_SIZE = 16;
	constexpr int CHUNK_HEIGHT = 64;
	constexpr int PIXELS_PER_TEXTURE = 16;
	constexpr float GRAVITY = 9.81;
	constexpr bool DEBUG_MODE = true;
	constexpr int SCREEN_WIDTH = 1600;
	constexpr int SCREEN_HEIGHT = 1200;
	constexpr bool SUNLIGHT_ON = false;
	constexpr bool SHOW_FPS = true;
	constexpr bool AO_ENABLED = true;
	constexpr bool TOGGLE_FLY = false;
	inline siv::PerlinNoise::seed_type WOLRD_SEED = DEBUG_MODE ? 1000 :rand() % 1000;
}