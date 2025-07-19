#include "chunk.hpp"

#include <cmath>
#include <format>
#include <stdexcept>

#include "game.hpp"
#include "raymath.h"
#include "stb_perlin.h"
#include "utilityStructures.hpp"

float fBm(const float x, const float z, const int octaves, const float lacunarity, const float gain,
          const int seed) {
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float sum = 0.0f;
    for (int i = 0; i < octaves; i++) {
        sum += amplitude *
               stb_perlin_noise3_seed(x * frequency, z * frequency, 0.0f, 0, 0, 0, seed + i);
        amplitude *= gain;
        frequency *= lacunarity;
    }
    return sum;
}

float computeMaxAmplitude(const int octaves, const float gain) {
    float a = 1.0f;
    float maxAmp = 0.0f;
    for (int i = 0; i < octaves; i++) {
        maxAmp += a;
        a *= gain;
    }
    return maxAmp;
}

int getHeight(const int x, const int z, const int seed) {
    constexpr int octaves = 6;
    constexpr float lacunarity = 2.0f;
    constexpr float gain = 0.5f;
    constexpr float noiseScale = 0.005f;  // tweak as needed

    // raw fBM in [ -maxAmp, +maxAmp ]
    const float raw = fBm(static_cast<float>(x) * noiseScale, static_cast<float>(z) * noiseScale,
                          octaves, lacunarity, gain, seed);

    // normalize to [-1,1]
    const float maxAmp = computeMaxAmplitude(octaves, gain);
    const float n = raw / maxAmp;

    // normalize to [0,1]
    float normalized = (n + 1.0f) * 0.5f;

    //   Option A: linear map to [0, maxWorldHeight]
    // const float maxWorldHeight = 200.0f;
    // float h = normalized * maxWorldHeight;

    //   Option B: exponential for spikier relief
    // float h = std::pow(normalized, 1.5f) * maxWorldHeight;

    //   Option C: mix linear + exponent
    const float heightLinear = normalized * 80.0f;                          // [0, 80]
    const float heightExponentiated = std::pow(normalized, 3.0f) * 120.0f;  // [0, 120]
    const float height =
        heightLinear * (1 - normalized) + heightExponentiated * normalized + 4;  // [4, 124]

    return static_cast<int>(std::ceil(height));
}

void Chunk::generate(const int seed, const int mapHeight) {
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            const int realHeight = getHeight(localToGlobalX(x), localToGlobalZ(z), seed);

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
}

void Chunk::generateTransforms(const Chunk* positiveXNeighbor, const Chunk* negativeXNeighbor,
                               const Chunk* positiveYNeighbor, const Chunk* negativeYNeighbor,
                               const Chunk* positiveZNeighbor, const Chunk* negativeZNeighbor) {
    auto dataWithSentinel = [this, &positiveXNeighbor, &negativeXNeighbor, &positiveYNeighbor,
                             &negativeYNeighbor, &positiveZNeighbor, &negativeZNeighbor](
                                const int x, const int y, const int z) -> BlockType {
        if (x >= 0 && x < CHUNK_SIZE && y >= 0 && y < CHUNK_SIZE && z >= 0 && z < CHUNK_SIZE) {
            return data_[x][y][z];
        }

        if (x < 0 && negativeXNeighbor) return negativeXNeighbor->data_[CHUNK_SIZE - 1][y][z];
        if (x >= CHUNK_SIZE && positiveXNeighbor) return positiveXNeighbor->data_[0][y][z];
        if (y < 0 && negativeYNeighbor) return negativeYNeighbor->data_[x][CHUNK_SIZE - 1][z];
        if (y >= CHUNK_SIZE && positiveYNeighbor) return positiveYNeighbor->data_[x][0][z];
        if (z < 0 && negativeZNeighbor) return negativeZNeighbor->data_[x][y][CHUNK_SIZE - 1];
        if (z >= CHUNK_SIZE && positiveZNeighbor) return positiveZNeighbor->data_[x][y][0];

        // No chunk there
        return BlockType::BLOCK_AIR;
    };

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                if (data_[x][y][z] == BlockType::BLOCK_AIR) {
                    continue;  // Skip air blocks
                }

                constexpr std::array<Vector3Integer, 6> neighborCubesOffsets = {
                    {Vector3Integer{-1, 0, 0}, Vector3Integer{1, 0, 0}, Vector3Integer{0, -1, 0},
                     Vector3Integer{0, 1, 0}, Vector3Integer{0, 0, -1}, Vector3Integer{0, 0, 1}}};

                bool isVisibleBlock = false;
                for (const Vector3Integer& offset : neighborCubesOffsets) {
                    const int neighborX = x + static_cast<int>(offset.x);
                    const int neighborY = y + static_cast<int>(offset.y);
                    const int neighborZ = z + static_cast<int>(offset.z);

                    // If the neighboring block is air, we need to render it
                    if (dataWithSentinel(neighborX, neighborY, neighborZ) == BlockType::BLOCK_AIR) {
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
