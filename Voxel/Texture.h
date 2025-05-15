#pragma once
#include <GL/glew.h>
#include <string>
#include <glm.hpp>
#include <array>
#include <unordered_map>

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

	static std::array<glm::vec2, 4> GetTileUVs(int tileX, int tileY, int tilesPerRow = 16, int tilesPerCol = 16);
	static void InitializeBlockTextures();
	static const BlockTextureSet& GetBlockTexture(uint8_t blockType);

private:
	float tileSize = 1.0f / 16.0f;  // 0.0625f
	GLuint textureID;
	int width, height, channels;
	std::string filepath;
	unsigned char* imageData = nullptr;
	static std::unordered_map<uint8_t, BlockTextureSet> blockTextures;
};