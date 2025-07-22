#include "Chunk.hpp"

#include <cmath>
#include <format>
#include <stdexcept>

#include "Game.hpp"
#include "Perf.hpp"
#include "raymath.h"
#include "stb_perlin.h"

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

int getHeight(const int x, const int z, const int seed, const int maxWorldHeight) {
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
    const float normalized = (n + 1.0f) * 0.5f;

    //   Option A: linear map to [0, maxWorldHeight]
    // float h = normalized * maxWorldHeight;

    //   Option B: exponential for spikier relief
    const float height = std::pow(normalized, 4.0f) * static_cast<float>(maxWorldHeight);

    //   Option C: mix linear + exponent
    // const float heightLinear = normalized * 80.0f;                          // [0, 80]
    // const float heightExponentiated = std::pow(normalized, 3.0f) * 120.0f;  // [0, 120]
    // const float height =
    //     heightLinear * (1 - normalized) + heightExponentiated * normalized + 4;  // [4, 124]

    return static_cast<int>(std::ceil(height));
}

void Chunk::generate(const int seed, const int maxHeight) {
    // static uint64_t globalIterations = 0;
    // const uint64_t firstMeasuredIteration = 10000;
    // const uint64_t nbMeasurements = 1e8;
    // uint64_t startCycles = 0;

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            const int realHeight = getHeight(localToGlobalX(x), localToGlobalZ(z), seed, maxHeight);

            if (localToGlobalY(0) >= realHeight) {
                continue;
            }

            // const bool isMeasured = globalIterations >= firstMeasuredIteration &&
            //                         globalIterations < firstMeasuredIteration + nbMeasurements;
            // globalIterations++;
            // if (isMeasured) {
            //     perfNbIterations++;
            //     startCycles = get_cycles();
            // }

            const int lastY = std::min(realHeight - localToGlobalY(0), CHUNK_SIZE);
            for (int localY = 0; localY < lastY; localY++) {
                const int globalYToRealHeight = localToGlobalY(localY);
                if (globalYToRealHeight <= realHeight - 4) {
                    data_[x][localY][z] = BlockType::BLOCK_STONE;
                } else if (globalYToRealHeight <= realHeight - 2) {
                    data_[x][localY][z] = BlockType::BLOCK_DIRT;
                } else {
                    data_[x][localY][z] = BlockType::BLOCK_GRASS;
                }
            }

            // if (isMeasured) {
            //     perfNbCycles += get_cycles() - startCycles;
            // }
        }
    }
}

void Chunk::generateTransforms(const OptionalRef<Chunk> adjacentChunkPositiveX,
                               const OptionalRef<Chunk> adjacentChunkNegativeX,
                               const OptionalRef<Chunk> adjacentChunkPositiveY,
                               const OptionalRef<Chunk> adjacentChunkNegativeY,
                               const OptionalRef<Chunk> adjacentChunkPositiveZ,
                               const OptionalRef<Chunk> adjacentChunkNegativeZ) {
    if (areTransformsFullyGenerated_) {
        return;
    }

    stoneTransforms.clear();
    dirtTransforms.clear();
    grassTransforms.clear();

    auto dataWithSentinel = [&](const int x, const int y, const int z) -> BlockType {
        if (x >= 0 && x < CHUNK_SIZE && y >= 0 && y < CHUNK_SIZE && z >= 0 && z < CHUNK_SIZE) {
            return data_[x][y][z];
        }

        if (x < 0 && adjacentChunkNegativeX)
            return adjacentChunkNegativeX->get().data_[CHUNK_SIZE - 1][y][z];
        if (x >= CHUNK_SIZE && adjacentChunkPositiveX)
            return adjacentChunkPositiveX->get().data_[0][y][z];
        if (y < 0 && adjacentChunkNegativeY)
            return adjacentChunkNegativeY->get().data_[x][CHUNK_SIZE - 1][z];
        if (y >= CHUNK_SIZE && adjacentChunkPositiveY)
            return adjacentChunkPositiveY->get().data_[x][0][z];
        if (z < 0 && adjacentChunkNegativeZ)
            return adjacentChunkNegativeZ->get().data_[x][y][CHUNK_SIZE - 1];
        if (z >= CHUNK_SIZE && adjacentChunkPositiveZ)
            return adjacentChunkPositiveZ->get().data_[x][y][0];

        // No chunk there
        return BlockType::BLOCK_STONE;  // Solid block to avoid rendering
    };

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                if (data_[x][y][z] == BlockType::BLOCK_AIR) {
                    continue;  // Skip air blocks
                }

                auto isVisible = [&](const int dx, const int dy, const int dz) -> bool {
                    const int neighborX = x + dx;
                    const int neighborY = y + dy;
                    const int neighborZ = z + dz;

                    return dataWithSentinel(neighborX, neighborY, neighborZ) ==
                           BlockType::BLOCK_AIR;
                };

                const bool isVisibleBlock = isVisible(+1, 0, 0) ||  // Positive X
                                            isVisible(-1, 0, 0) ||  // Negative X
                                            isVisible(0, +1, 0) ||  // Positive Y
                                            isVisible(0, -1, 0) ||  // Negative Y
                                            isVisible(0, 0, +1) ||  // Positive Z
                                            isVisible(0, 0, -1);    // Negative Z

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

    if (adjacentChunkPositiveX && adjacentChunkNegativeX && adjacentChunkPositiveY &&
        adjacentChunkNegativeY && adjacentChunkPositiveZ && adjacentChunkNegativeZ) {
        areTransformsFullyGenerated_ = true;
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
