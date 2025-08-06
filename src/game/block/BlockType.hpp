#pragma once
#include <cstdint>

#include "common/UtilityStructures.hpp"
#include "raylib.h"

struct Vertex {
    Vector3Int position;
    Vector3Int normal;
    Vector2 textureCoord;
};

enum class BlockType : uint8_t {
    BLOCK_AIR = 0,
    BLOCK_GRASS = 1,
    BLOCK_DIRT = 2,
    BLOCK_STONE = 3,
    BLOCK_SAND = 4,
    BLOCK_WATER = 5,
};