#include "Chunk.hpp"

#include <cmath>
#include <format>

#include "Game.hpp"
#include "PerlinNoise.hpp"
#include "raymath.h"

float fBm(const float x, const float y, const int octaves, const float lacunarity, const float gain,
          const int seed) {
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float sum = 0.0f;
    for (int i = 0; i < octaves; i++) {
        sum += amplitude * stb_perlin_noise2_seed(x * frequency, y * frequency, seed + i);
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

int getHeight(const int x, const int y, const int seed, const int maxWorldHeight) {
    constexpr int octaves = 6;
    constexpr float lacunarity = 2.0f;
    constexpr float gain = 0.5f;
    constexpr float noiseScale = 0.005f;  // tweak as needed

    // raw fBM in [ -maxAmp, +maxAmp ]
    const float raw = fBm(static_cast<float>(x) * noiseScale, static_cast<float>(y) * noiseScale,
                          octaves, lacunarity, gain, seed);

    // normalize to [-1,1]
    const float maxAmp = computeMaxAmplitude(octaves, gain);
    const float n = raw / maxAmp;

    // normalize to [0,1]
    const float normalized = (n + 1.0f) * 0.5f;

    //   Option A: linear map to [0, maxWorldHeight]
    // const float height = normalized * maxWorldHeight;

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
        for (int y = 0; y < CHUNK_SIZE; y++) {
            const int realHeight = getHeight(localToGlobalX(x), localToGlobalY(y), seed, maxHeight);

            if (localToGlobalZ(0) >= realHeight) {
                continue;
            }

            // const bool isMeasured = globalIterations >= firstMeasuredIteration &&
            //                         globalIterations < firstMeasuredIteration + nbMeasurements;
            // globalIterations++;
            // if (isMeasured) {
            //     perfNbIterations++;
            //     startCycles = get_cycles();
            // }

            constexpr int minGenerationHeight =
                24;  // if the terrain is too low, fill it with water

            const int lastZ =
                std::min(std::max(realHeight, minGenerationHeight) - localToGlobalZ(0), CHUNK_SIZE);
            for (int localZ = 0; localZ < lastZ; localZ++) {
                const int globalZToRealHeight = localToGlobalZ(localZ);
                if (globalZToRealHeight < minGenerationHeight) {
                    data_[x][y][localZ] = Block{BlockType::BLOCK_WATER};
                } else if (globalZToRealHeight < minGenerationHeight + 2) {
                    data_[x][y][localZ] = Block{BlockType::BLOCK_SAND};
                } else if (globalZToRealHeight <= realHeight - 4) {
                    data_[x][y][localZ] = Block{BlockType::BLOCK_STONE};
                } else if (globalZToRealHeight <= realHeight - 2) {
                    data_[x][y][localZ] = Block{BlockType::BLOCK_DIRT};
                } else if (globalZToRealHeight < realHeight) {
                    data_[x][y][localZ] = Block{BlockType::BLOCK_GRASS};
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

    meshVerts_.clear(), meshNorms_.clear(), meshUVs_.clear(), meshIndices_.clear();
    chunkMesh_ = {};

    auto dataWithSentinel = [&](const int x, const int y, const int z) -> Block {
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
        return Block{BlockType::BLOCK_STONE};  // Solid block to avoid rendering
    };

    std::vector<Vertex> vertices;

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                const Block& block = data_[x][y][z];

                if (!block.isRendered()) continue;

                auto isVisible = [&](const Vector3Int& neighbourOffset) -> bool {
                    return !dataWithSentinel(x + neighbourOffset.x, y + neighbourOffset.y,
                                             z + neighbourOffset.z)
                                .isRendered();
                };

                const std::array isFaceVisible = {
                    isVisible({1, 0, 0}),   // +X
                    isVisible({-1, 0, 0}),  // -X
                    isVisible({0, 1, 0}),   // +Y
                    isVisible({0, -1, 0}),  // -Y
                    isVisible({0, 0, 1}),   // +Z
                    isVisible({0, 0, -1})   // -Z
                };

                block.generateBlockMesh({x, y, z}, vertices, meshIndices_, isFaceVisible,
                                        textureAtlas());
            }
        }
    }

    meshVerts_.reserve(vertices.size() * 3);
    meshNorms_.reserve(vertices.size() * 3);
    meshUVs_.reserve(vertices.size() * 2);

    for (const auto& vertice : vertices) {
        meshVerts_.push_back(static_cast<float>(vertice.position.x));
        meshVerts_.push_back(static_cast<float>(vertice.position.y));
        meshVerts_.push_back(static_cast<float>(vertice.position.z));

        meshNorms_.push_back(static_cast<float>(vertice.normal.x));
        meshNorms_.push_back(static_cast<float>(vertice.normal.y));
        meshNorms_.push_back(static_cast<float>(vertice.normal.z));

        meshUVs_.push_back(vertice.textureCoord.x);
        meshUVs_.push_back(vertice.textureCoord.y);
    }

    chunkMesh_.vertexCount = static_cast<int>(vertices.size());
    chunkMesh_.triangleCount = static_cast<int>(meshIndices_.size()) / 3;
    chunkMesh_.vertices = meshVerts_.data();
    chunkMesh_.normals = meshNorms_.data();
    chunkMesh_.texcoords = meshUVs_.data();
    chunkMesh_.indices = meshIndices_.data();

    UploadMesh(&chunkMesh_, false);

    if (adjacentChunkPositiveX && adjacentChunkNegativeX && adjacentChunkPositiveY &&
        adjacentChunkNegativeY && adjacentChunkPositiveZ && adjacentChunkNegativeZ) {
        areTransformsFullyGenerated_ = true;
    }
}

void Chunk::render() const {
    if (meshIndices_.empty()) return;

    const Matrix pos = MatrixTranslate(static_cast<float>(localToGlobalX(0)),
                                       static_cast<float>(localToGlobalY(0)),
                                       static_cast<float>(localToGlobalZ(0)));
    DrawMesh(chunkMesh_, materialAtlas_, pos);
}
