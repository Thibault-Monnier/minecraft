#include "Chunk.hpp"

#include <cmath>
#include <format>
#include <stdexcept>

#include "Game.hpp"
#include "Perf.hpp"
#include "TextureAtlas.hpp"
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

            constexpr int minGenerationHeight =
                24;  // if the terrain is too low, fill it with water

            const int lastY =
                std::min(std::max(realHeight, minGenerationHeight) - localToGlobalY(0), CHUNK_SIZE);
            for (int localY = 0; localY < lastY; localY++) {
                const int globalYToRealHeight = localToGlobalY(localY);
                if (globalYToRealHeight < minGenerationHeight) {
                    data_[x][localY][z] = BlockType::BLOCK_WATER;
                } else if (globalYToRealHeight < minGenerationHeight + 2) {
                    data_[x][localY][z] = BlockType::BLOCK_SAND;
                } else if (globalYToRealHeight <= realHeight - 4) {
                    data_[x][localY][z] = BlockType::BLOCK_STONE;
                } else if (globalYToRealHeight <= realHeight - 2) {
                    data_[x][localY][z] = BlockType::BLOCK_DIRT;
                } else if (globalYToRealHeight < realHeight) {
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

    meshVerts_.clear(), meshNorms_.clear(), meshUVs_.clear(), meshIndices_.clear();
    chunkMesh_ = {};

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

    constexpr Rectangle textureRectGrass{0, 0, TEXTURE_SIZE, TEXTURE_SIZE};
    constexpr Rectangle textureRectDirt{TEXTURE_SIZE, 0, TEXTURE_SIZE, TEXTURE_SIZE};
    constexpr Rectangle textureRectStone{TEXTURE_SIZE * 2, 0, TEXTURE_SIZE, TEXTURE_SIZE};
    constexpr Rectangle textureRectSand{TEXTURE_SIZE * 3, 0, TEXTURE_SIZE, TEXTURE_SIZE};
    constexpr Rectangle textureRectWater{TEXTURE_SIZE * 4, 0, TEXTURE_SIZE, TEXTURE_SIZE};

    struct Vertex {
        Vector3Int position;
        Vector3Int normal;
        Vector2 textureCoord;
    };

    std::vector<Vertex> vertices;

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                if (data_[x][y][z] == BlockType::BLOCK_AIR) {
                    continue;
                }

                auto appendQuad = [&](const Vector3Int& origin, const Vector3Int& edgeDirU,
                                      const Vector3Int& edgeDirV, const Vector3Int& faceNormal,
                                      const Rectangle& textureRect) {
                    const int startIndex = static_cast<int>(vertices.size());

                    const float u0 = textureRect.x / static_cast<float>(textureAtlas().width);
                    const float v0 = textureRect.y / static_cast<float>(textureAtlas().height);
                    const float u1 = (textureRect.x + textureRect.width) /
                                     static_cast<float>(textureAtlas().width);
                    const float v1 = (textureRect.y + textureRect.height) /
                                     static_cast<float>(textureAtlas().height);

                    const Vector2 textureCoordBL{u0, v0};
                    const Vector2 textureCoordBR{u1, v0};
                    const Vector2 textureCoordTR{u1, v1};
                    const Vector2 textureCoordTL{u0, v1};

                    vertices.push_back({origin, faceNormal, textureCoordBL});
                    vertices.push_back({origin + edgeDirU, faceNormal, textureCoordBR});
                    vertices.push_back({origin + edgeDirU + edgeDirV, faceNormal, textureCoordTR});
                    vertices.push_back({origin + edgeDirV, faceNormal, textureCoordTL});

                    meshIndices_.insert(
                        meshIndices_.end(),
                        {static_cast<uint16_t>(startIndex), static_cast<uint16_t>(startIndex + 1),
                         static_cast<uint16_t>(startIndex + 2), static_cast<uint16_t>(startIndex),
                         static_cast<uint16_t>(startIndex + 2),
                         static_cast<uint16_t>(startIndex + 3)});
                };

                struct Face {
                    Vector3Int neighbourOffset;
                    Vector3Int originOffset;  // corner where u=0,v=0
                    Vector3Int u, v;          // edge vectors
                    Vector3Int normal;
                };

                auto isVisible = [&](const Vector3Int& neighbourOffset) -> bool {
                    return dataWithSentinel(x + neighbourOffset.x, y + neighbourOffset.y,
                                            z + neighbourOffset.z) == BlockType::BLOCK_AIR;
                };

                Rectangle textureRect;
                const BlockType blockType = data_[x][y][z];
                if (blockType == BlockType::BLOCK_GRASS)
                    textureRect = textureRectGrass;
                else if (blockType == BlockType::BLOCK_DIRT)
                    textureRect = textureRectDirt;
                else if (blockType == BlockType::BLOCK_STONE)
                    textureRect = textureRectStone;
                else if (blockType == BlockType::BLOCK_SAND)
                    textureRect = textureRectSand;
                else if (blockType == BlockType::BLOCK_WATER)
                    textureRect = textureRectWater;
                else {
                    throw std::runtime_error(
                        "Unknown block type encountered during mesh generation.");
                }

                static constexpr Face faces[6] = {
                    // +X (east)
                    {{1, 0, 0}, {1, 0, 1}, {0, 0, -1}, {0, 1, 0}, {1, 0, 0}},
                    // -X (west)
                    {{-1, 0, 0}, {0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {-1, 0, 0}},

                    // +Y (top)
                    {{0, 1, 0}, {1, 1, 0}, {-1, 0, 0}, {0, 0, 1}, {0, 1, 0}},
                    // -Y (bottom)
                    {{0, -1, 0}, {0, 0, 0}, {1, 0, 0}, {0, 0, 1}, {0, -1, 0}},

                    // +Z (back)
                    {{0, 0, 1}, {0, 0, 1}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}},
                    // -Z (front)
                    {{0, 0, -1}, {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, 0, -1}},
                };

                for (const auto& face : faces) {
                    if (isVisible(face.neighbourOffset)) {
                        const Vector3Int origin = {x + face.originOffset.x, y + face.originOffset.y,
                                                   z + face.originOffset.z};
                        appendQuad(origin, face.u, face.v, face.normal, textureRect);
                    }
                }
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
