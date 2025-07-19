#include "chunk.hpp"

#include <cmath>
#include <format>
#include <iostream>
#include <stdexcept>

#include "game.hpp"
#include "raymath.h"
#include "stb_perlin.h"

void Chunk::generate(const int seed, const int mapHeightChunks) {
    const float noiseScale = 0.01f;  // controls noise “zoom”

    const int mapHeight = mapHeightChunks * CHUNK_SIZE;

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            float height = stb_perlin_noise3_seed(
                static_cast<float>(localToGlobalX(x)) * noiseScale,
                static_cast<float>(localToGlobalZ(z)) * noiseScale, 0.0f, 0, 0, 0, seed);
            height = (height + 1.0f) / 2.0f;                  // Normalize to (0, 1)
            height = height * static_cast<float>(mapHeight);  // Scale height
            int realHeight = std::ceil(height);  // Round to next integer in (0, mapHeight]

            for (int globalY = localToGlobalY(0); globalY < mapHeight; globalY++) {
                const int localY = globalToLocalY(globalY);

                if (localY >= CHUNK_SIZE) break;

                if (globalY < realHeight - 4)
                    data_[x][localY][z] = BlockType::BLOCK_STONE;  // Stone for lower layers
                else if (globalY < realHeight - 1)
                    data_[x][localY][z] = BlockType::BLOCK_DIRT;  // Grass for upper layers
                else if (globalY == realHeight - 1)
                    data_[x][localY][z] = BlockType::BLOCK_GRASS;  // Grass on top
                else {
                    data_[x][localY][z] = BlockType::BLOCK_AIR;  // Air above the terrain
                }
            }
        }
    }

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                if (data_[x][y][z] == BlockType::BLOCK_AIR) {
                    continue;  // Skip air blocks
                }

                constexpr std::array<Vector3, 6> neighbourCubesOffsets = {
                    {Vector3{-1.0f, 0.0f, 0.0f}, Vector3{1.0f, 0.0f, 0.0f},
                     Vector3{0.0f, -1.0f, 0.0f}, Vector3{0.0f, 1.0f, 0.0f},
                     Vector3{0.0f, 0.0f, -1.0f}, Vector3{0.0f, 0.0f, 1.0f}}};

                bool isVisibleBlock = false;
                for (const Vector3& offset : neighbourCubesOffsets) {
                    const int neighbourX = x + static_cast<int>(offset.x);
                    const int neighbourY = y + static_cast<int>(offset.y);
                    const int neighbourZ = z + static_cast<int>(offset.z);

                    // If the neighbouring block is air or the block is on the edge of the chunk, we
                    // need to render it
                    if (neighbourX < 0 || neighbourX >= CHUNK_SIZE || neighbourY < 0 ||
                        neighbourY >= CHUNK_SIZE || neighbourZ < 0 || neighbourZ >= CHUNK_SIZE ||
                        data_[neighbourX][neighbourY][neighbourZ] == BlockType::BLOCK_AIR) {
                        isVisibleBlock = true;
                        break;
                    }
                }

                if (!isVisibleBlock) {
                    continue;
                }

                Matrix model = MatrixTranslate(static_cast<float>(localToGlobalX(x)) + 0.5f,
                                               static_cast<float>(localToGlobalY(y)) + 0.5f,
                                               static_cast<float>(localToGlobalZ(z)) + 0.5f);

                if (data_[x][y][z] == BlockType::BLOCK_GRASS) {
                    grassTransforms.push_back(model);
                } else if (data_[x][y][z] == BlockType::BLOCK_DIRT) {
                    dirtTransforms.push_back(model);
                } else if (data_[x][y][z] == BlockType::BLOCK_STONE) {
                    stoneTransforms.push_back(model);
                } else {
                    throw std::runtime_error(
                        std::format("Unknown block type at ({}, {}, {}): got {}", x, y, z,
                                    static_cast<int>(data_[x][y][z])));
                }
            }
        }
    }
}

void Chunk::render() const {
    if (!grassTransforms.empty())
        DrawMeshInstanced(cubeMesh_, materialGrass_, grassTransforms.data(),
                          static_cast<int>(grassTransforms.size()));
    if (!dirtTransforms.empty())
        DrawMeshInstanced(cubeMesh_, materialDirt_, dirtTransforms.data(),
                          static_cast<int>(dirtTransforms.size()));
    if (!stoneTransforms.empty())
        DrawMeshInstanced(cubeMesh_, materialStone_, stoneTransforms.data(),
                          static_cast<int>(stoneTransforms.size()));
}
