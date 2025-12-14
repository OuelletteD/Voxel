#pragma once
#include <GL/glew.h>
#include <string>
#include <glm/glm.hpp>
#include <array>
#include <unordered_map>
#include "Config.h"
#include "BlockType.h"

struct BlockTextureSet {
	std::array<glm::vec2, 4> top;
	std::array<glm::vec2, 4> bottom;
	std::array<glm::vec2, 4> side;
};
class Texture {

public:
	Texture(const std::string& filepath);
	~Texture();

	bool Initialize();
	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	static std::array<glm::vec2, 4> GetTileUVs(int tileX, int tileY, int tilesPerRow = Config::PIXELS_PER_TEXTURE, int tilesPerCol = Config::PIXELS_PER_TEXTURE);
	static void InitializeBlockTextures();
	static const BlockTextureSet& GetBlockTexture(BlockType blockType);

private:
	float tileSize = 1.0f / (float)Config::PIXELS_PER_TEXTURE;  // 0.0625f
	GLuint textureID;
	int width, height, channels;
	std::string filepath;
	unsigned char* imageData = nullptr;
	static std::unordered_map<uint8_t, BlockTextureSet> blockTextures;
};