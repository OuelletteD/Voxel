#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "ErrorLogger.h"

Texture::Texture(const std::string& filepath)

    : textureID(0), width(0), height(0), channels(0), filepath(filepath)
{
    stbi_set_flip_vertically_on_load(true);

    imageData = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
    if (!imageData) {
        ErrorLogger::LogError("Failed to load texture: " + filepath);
        return;
    }
}
bool Texture::Initialize(){
    if (!imageData) {
        ErrorLogger::LogError("No image data to initialize texture from.");
        return false;
    }
    glGenTextures(1, &textureID);
    if (textureID == 0) {
        ErrorLogger::LogError("Failed to generate OpenGL texture.");
        stbi_image_free(imageData);
        imageData = nullptr;
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 4);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(imageData);

    imageData = nullptr;
    return true;
}

std::array<glm::vec2, 4> Texture::GetTileUVs(int tileX, int tileY, int tilesPerRow, int tilesPerCol) {
    float tileSizeX = 1.0f / static_cast<float>(tilesPerRow);
    float tileSizeY = 1.0f / static_cast<float>(tilesPerCol);

    float uMin = tileX * tileSizeX;
    //float vMin = tileY * tileSizeY;
    float vMin = 1.0f - (tileY + 1) * tileSizeY;
    float uMax = uMin + tileSizeX;
    float vMax = vMin + tileSizeY;

    std::array<glm::vec2, 4> uvs = {
    glm::vec2(uMin, vMin), // Bottom-left
    glm::vec2(uMax, vMin), // Bottom-right
    glm::vec2(uMax, vMax), // Top-right
    glm::vec2(uMin, vMax)  // Top-left
    };

    return uvs;
}
std::unordered_map<uint8_t, BlockTextureSet> Texture::blockTextures;
void Texture::InitializeBlockTextures() {
    blockTextures[1] = {
        GetTileUVs(2,0),
        GetTileUVs(2,0),
        GetTileUVs(2,0)
    };
    blockTextures[2] = {
        GetTileUVs(0,0),
        GetTileUVs(2,0),
        GetTileUVs(3,0)
    };
    blockTextures[3] = {
        GetTileUVs(1,0),
        GetTileUVs(1,0),
        GetTileUVs(1,0)
    };
}

const BlockTextureSet& Texture::GetBlockTexture(uint8_t blockType) {
    return blockTextures[blockType];
}

Texture::~Texture() {
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
    }
    if (imageData) {
        stbi_image_free(imageData);
    }
}

void Texture::Bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void Texture::Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}