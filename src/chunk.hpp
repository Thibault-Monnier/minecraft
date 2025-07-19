#pragma once
#include <array>
#include <vector>

#include "raylib.h"

enum class BlockType { BLOCK_AIR, BLOCK_GRASS, BLOCK_DIRT, BLOCK_STONE };

class Chunk {
   public:
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

    void generate(int seed, int mapHeightChunks);
    void render() const;

    static constexpr int CHUNK_SIZE = 16;

   private:
    const int chunkX_;
    const int chunkY_;
    const int chunkZ_;

    const Shader instancedShader_;
    const Mesh cubeMesh_;
    const Material materialGrass_;
    const Material materialDirt_;
    const Material materialStone_;

    std::vector<Matrix> grassTransforms;
    std::vector<Matrix> dirtTransforms;
    std::vector<Matrix> stoneTransforms;

    typedef std::array<std::array<std::array<BlockType, CHUNK_SIZE>, CHUNK_SIZE>, CHUNK_SIZE>
        ChunkData;

    ChunkData data_{};  // 3D array to hold the block types in the chunk

    [[nodiscard]] int localToGlobalX(const int x) const { return chunkX_ * CHUNK_SIZE + x; }
    [[nodiscard]] int localToGlobalY(const int y) const { return chunkY_ * CHUNK_SIZE + y; }
    [[nodiscard]] int localToGlobalZ(const int z) const { return chunkZ_ * CHUNK_SIZE + z; }

    [[nodiscard]] int globalToLocalY(const int y) const { return y - chunkY_ * CHUNK_SIZE; }
};