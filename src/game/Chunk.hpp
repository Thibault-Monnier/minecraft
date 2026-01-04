#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "block/Block.hpp"
#include "common/UtilityTypes.hpp"
#include "raylib.h"

class Chunk {
   public:
    static constexpr int CHUNK_SIZE = 32;

    Chunk(const int x, const int y, const int z, const Material& materialAtlas)
        : chunkX_(x), chunkY_(y), chunkZ_(z), materialAtlas_(materialAtlas) {}

    Chunk(Chunk&& other) noexcept = delete;
    Chunk& operator=(Chunk&&) noexcept = delete;

    Chunk(const Chunk& other) = delete;
    Chunk& operator=(const Chunk&) = delete;

    ~Chunk() = default;

    [[nodiscard]] int getX() const { return chunkX_; }
    [[nodiscard]] int getY() const { return chunkY_; }
    [[nodiscard]] int getZ() const { return chunkZ_; }

    [[nodiscard]] static Vector3 getCenterPosition(const int chunkX, const int chunkY,
                                                   const int chunkZ) {
        return {(static_cast<float>(chunkX) + 0.5f) * CHUNK_SIZE,
                (static_cast<float>(chunkY) + 0.5f) * CHUNK_SIZE,
                (static_cast<float>(chunkZ) + 0.5f) * CHUNK_SIZE};
    }

    [[nodiscard]] Vector3 getCenterPosition() const {
        return getCenterPosition(chunkX_, chunkY_, chunkZ_);
    }

    void generate(int seed, int maxHeight);

    void generateTransforms(OptionalRef<Chunk> adjacentChunkPositiveX,
                            OptionalRef<Chunk> adjacentChunkNegativeX,
                            OptionalRef<Chunk> adjacentChunkPositiveY,
                            OptionalRef<Chunk> adjacentChunkNegativeY,
                            OptionalRef<Chunk> adjacentChunkPositiveZ,
                            OptionalRef<Chunk> adjacentChunkNegativeZ);

    void render() const;

    typedef std::array<std::array<std::array<Block, CHUNK_SIZE>, CHUNK_SIZE>, CHUNK_SIZE> ChunkData;

    [[nodiscard]] const ChunkData& getData() const { return data_; }

   private:
    const int chunkX_;
    const int chunkY_;
    const int chunkZ_;

    const Material& materialAtlas_;

    std::vector<float> meshVerts_, meshNorms_, meshUVs_;
    std::vector<uint16_t> meshIndices_;

    Mesh chunkMesh_{};

    bool areTransformsFullyGenerated_ = false;

    ChunkData data_;  // 3D array to hold the block types in the chunk

    [[nodiscard]] int localToGlobalX(const int x) const { return chunkX_ * CHUNK_SIZE + x; }
    [[nodiscard]] int localToGlobalY(const int y) const { return chunkY_ * CHUNK_SIZE + y; }
    [[nodiscard]] int localToGlobalZ(const int z) const { return chunkZ_ * CHUNK_SIZE + z; }

    [[nodiscard]] Texture2D& textureAtlas() const {
        return materialAtlas_.maps[MATERIAL_MAP_DIFFUSE].texture;
    }
};