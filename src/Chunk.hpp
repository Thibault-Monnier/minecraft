#pragma once
#include <array>
#include <vector>

#include "raylib.h"

enum class BlockType { BLOCK_AIR, BLOCK_GRASS, BLOCK_DIRT, BLOCK_STONE };

class Chunk {
   public:
    static constexpr int CHUNK_SIZE = 32;

    Chunk(const int x, const int y, const int z, const Shader& shader, const Mesh& cubeMesh,
          const Material& materialGrass, const Material& materialDirt,
          const Material& materialStone)
        : chunkX_(x),
          chunkY_(y),
          chunkZ_(z),
          instancedShader_(shader),
          cubeMesh_(cubeMesh),
          materialGrass_(materialGrass),
          materialDirt_(materialDirt),
          materialStone_(materialStone) {}

    [[nodiscard]] int getX() const { return chunkX_; }
    [[nodiscard]] int getY() const { return chunkY_; }
    [[nodiscard]] int getZ() const { return chunkZ_; }

    [[nodiscard]] Vector3 getCenterPosition() const {
        return {(static_cast<float>(chunkX_) + 0.5f) * CHUNK_SIZE,
                (static_cast<float>(chunkY_) + 0.5f) * CHUNK_SIZE,
                (static_cast<float>(chunkZ_) + 0.5f) * CHUNK_SIZE};
    }

    void generate(int seed, int maxHeight);
    void generateTransforms(const Chunk* positiveXNeighbor, const Chunk* negativeXNeighbor,
                            const Chunk* positiveYNeighbor, const Chunk* negativeYNeighbor,
                            const Chunk* positiveZNeighbor, const Chunk* negativeZNeighbor);

    void render() const;

    typedef std::array<std::array<std::array<BlockType, CHUNK_SIZE>, CHUNK_SIZE>, CHUNK_SIZE>
        ChunkData;

    [[nodiscard]] const ChunkData& getData() const { return data_; }

   private:
    const int chunkX_;
    const int chunkY_;
    const int chunkZ_;

    const Shader& instancedShader_;
    const Mesh& cubeMesh_;
    const Material& materialGrass_;
    const Material& materialDirt_;
    const Material& materialStone_;

    std::vector<Matrix> grassTransforms;
    std::vector<Matrix> dirtTransforms;
    std::vector<Matrix> stoneTransforms;

    ChunkData data_{};  // 3D array to hold the block types in the chunk

    [[nodiscard]] int localToGlobalX(const int x) const { return chunkX_ * CHUNK_SIZE + x; }
    [[nodiscard]] int localToGlobalY(const int y) const { return chunkY_ * CHUNK_SIZE + y; }
    [[nodiscard]] int localToGlobalZ(const int z) const { return chunkZ_ * CHUNK_SIZE + z; }

    [[nodiscard]] int globalToLocalY(const int y) const { return y - chunkY_ * CHUNK_SIZE; }
};