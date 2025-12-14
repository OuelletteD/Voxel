#include "RenderingMath.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include "VoxelRaycaster.h"

float calculateAOFactor(int faceIndex, int cornerIndex, glm::ivec3 position, std::function<BlockType(glm::ivec3)> isSolidAt) {
    const AOOffsets& offsets = aoTable[faceIndex][cornerIndex];


    glm::ivec3 side1Pos = position + offsets.side1;
    glm::ivec3 side2Pos = position + offsets.side2;
    glm::ivec3 cornerPos = position + offsets.corner;
    float side1 = returnOcclusionWeight((BlockType)isSolidAt(side1Pos));
    float side2 = returnOcclusionWeight((BlockType)isSolidAt(side2Pos));
    float corner = returnOcclusionWeight((BlockType)isSolidAt(cornerPos));
    if (side1 >= 1.0f && side2 >= 1.0f) {
        return 0.0f;
    }
    float occlusion = side1 + side2 + corner;
    return glm::clamp(1.0f - occlusion * 0.333f, 0.0f, 1.0f);
}

float returnOcclusionWeight(BlockType type) {
    switch (type) {
        case BlockType::Air:
            return 0.0f;
        case BlockType::Water:
            return 0.3f;
        default:
            return 1.0f;
    }
}

bool IsChunkInFrustum(const std::array<Plane, 6>& planes, const glm::vec3& min, const glm::vec3& max) {
    for (const Plane& plane : planes) {
        glm::vec3 p = min;
        if (plane.normal.x >= 0) p.x = max.x;
        if (plane.normal.y >= 0) p.y = max.y;
        if (plane.normal.z >= 0) p.z = max.z;

        if (plane.DistanceToPoint(p) < 0) {
            return false;
        }
    }
    return true;
}

AOOffsets aoTable[6][4] = {
    // 0: +Y (Top)
    {
        {{0, 1, -1}, {-1, 1, 0}, {-1, 1, -1}},  // top-left
        {{0, 1,  -1}, {1, 1, 0}, {1, 1,  -1}},  // top-right
        {{0, 1,  1}, {1, 1, 0}, {1, 1,  1}}, // bottom-right
        {{0, 1, 1}, {-1, 1, 0}, {-1, 1, 1}}, // bottom-left
    },
    // 1: -Y (Bottom)
    {
        {{0, -1, -1}, {-1, -1, 0}, {-1, -1, -1}},  // top-left (looking up at -Y)
        {{0, -1,  -1}, {1, -1, 0}, {1, -1,  -1}},  // top-right
        {{0, -1,  1}, {1, -1, 0}, {1, -1,  1}}, // bottom-right
        {{0, -1, 1}, {-1, -1, 0}, {-1, -1, 1}}, // bottom-left
    },
    // 2: +Z (Front)
    {
        {{-1, 0, 1}, {0, -1, 1}, {-1, -1, 1}}, // bottom-left (looking at +Z)
        {{1, 0, 1}, {0, -1, 1}, {1, -1, 1}}, // bottom-right
        {{1, 0, 1}, {0,1, 1}, {1,1, 1}}, // top-right
        {{-1, 0, 1}, {0,1, 1}, {-1,1, 1}}, // top-left
    },
    // 3: -Z (Back)
    {
        {{1, 0, -1}, {0, -1, -1}, {1, -1, -1}}, // bottom-right (looking at -Z)
        {{-1, 0, -1}, {0, -1, -1}, {-1, -1, -1}}, // bottom-left
        {{-1, 0, -1}, {0,1, -1}, {-1,1, -1}}, // top-left
        {{1, 0, -1}, {0,1, -1}, {1,1, -1}}, // top-right
    },
    // 4: -X (Left)
 {
        {{-1, -1, 0}, {-1, 0,  -1}, {-1, -1,  -1}}, // bottom-left (looking at from outside)
        {{-1, -1, 0}, {-1, 0, 1}, {-1, -1, 1}}, // bottom-right
        {{-1,1, 0}, {-1, 0, 1}, {-1,1, 1}}, // top-right
        {{-1,1, 0}, {-1, 0,  -1}, {-1,1,  -1}}, // top-left
    },
    // 5: +X (Right)
    {
        {{+1, -1, 0}, {+1, 0,  -1}, {+1, -1,  -1}} , //  bottom-right
        {{+1, -1, 0}, {+1, 0, 1}, {+1, -1, 1}}, // bottom-left (looking at +X)
        {{+1,1, 0}, {+1, 0, 1}, {+1,1, 1}}, // top-left
        {{+1,1, 0}, {+1, 0,  -1}, {+1,1,  -1}} // top-right
    },
};

glm::ivec3 faceDirections[6] = {
   {0, 1, 0},   // Top
   {0, -1, 0},  // Bottom
   {0, 0, 1},   // Front
   {0, 0, -1},  // Back
   {-1, 0, 0},  // Left
   {1, 0, 0}    // Right
};

glm::vec3 faceOffsets[6][4] = {
    { {-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f} },
    { {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, 0.5f}, {-0.5f, -0.5f, 0.5f} },
    { {-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f} },
    { {0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, -0.5f} },
    { {-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, -0.5f} },
    { {0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, -0.5f} }
};

