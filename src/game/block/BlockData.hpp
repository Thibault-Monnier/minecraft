#pragma once

#include <cstdint>

#include "BlockType.hpp"
#include "common/UtilityStructures.hpp"
#include "raylib.h"

struct BlockTypeData {
    const Vector3 size{};
    const Vector2Int textureAtlasPositionTop{};
    const Vector2Int textureAtlasPositionBottom{};
    const Vector2Int textureAtlasPositionSides{};
    const bool isRendered = true;
};

constexpr Vector3 FULL_BLOCK_SIZE{1.0f, 1.0f, 1.0f};

static constexpr BlockTypeData BLOCK_DATA[] = {
    {.size = FULL_BLOCK_SIZE, .isRendered = false},                           // BLOCK_AIR
    {FULL_BLOCK_SIZE, Vector2Int{0, 0}, Vector2Int{0, 0}, Vector2Int{0, 0}},  // BLOCK_GRASS
    {FULL_BLOCK_SIZE, Vector2Int{1, 0}, Vector2Int{1, 0}, Vector2Int{1, 0}},  // BLOCK_DIRT
    {FULL_BLOCK_SIZE, Vector2Int{2, 0}, Vector2Int{2, 0}, Vector2Int{2, 0}},  // BLOCK_STONE
    {FULL_BLOCK_SIZE, Vector2Int{3, 0}, Vector2Int{3, 0}, Vector2Int{3, 0}},  // BLOCK_SAND
    {FULL_BLOCK_SIZE, Vector2Int{4, 0}, Vector2Int{4, 0}, Vector2Int{4, 0}}   // BLOCK_WATER
};

inline const BlockTypeData& getBlockTypeData(const BlockType type) {
    return BLOCK_DATA[static_cast<std::underlying_type_t<BlockType>>(type)];
}