#pragma once
#include "PerlinNoise.hpp"

namespace Config {
	constexpr int CHUNK_SIZE = 16;
	constexpr int CHUNK_HEIGHT = 128;
	constexpr int PIXELS_PER_TEXTURE = 16;
	constexpr float GRAVITY = 9.81f;
	constexpr int SQRT_CHUNKS_TO_CREATE = 13;
	constexpr bool DEBUG_MODE = false;
	constexpr int SCREEN_WIDTH = 1600;
	constexpr int SCREEN_HEIGHT = 1200;
	constexpr bool SHOW_FPS = true;
	constexpr bool AO_ENABLED = true;
	constexpr bool TOGGLE_FLY = false;
	constexpr float FOV = 70.0f;
	constexpr int CHUNK_LOAD_RADIUS = 13;
	inline siv::PerlinNoise::seed_type WOLRD_SEED; //Set in Voxel.cpp
}