#pragma once

#include <vector>

#include "BlockData.hpp"
#include "BlockType.hpp"
#include "game/TextureAtlas.hpp"

class Block {
   public:
    Block() = default;
    explicit Block(const BlockType type) : type_(type) {}

    [[nodiscard]] static Block& stoneBlock() {
        static Block stoneBlockInstance(BlockType::BLOCK_STONE);
        return stoneBlockInstance;
    }

    [[nodiscard]] bool isRendered() const { return blockTypeData().isRendered; }
    [[nodiscard]] BlockType type() const { return type_; }

    // isFaceVisible faces in order: +X, -X, +Y, -Y, +Z, -Z
    void generateBlockMesh(const Vector3Int& position, std::vector<Vertex>& chunkMeshVerts,
                           std::vector<uint16_t>& chunkMeshIndices_,
                           const std::array<bool, 6>& isFaceVisible,
                           const Texture2D& textureAtlas) const;

   private:
    BlockType type_ = BlockType::BLOCK_AIR;

    [[nodiscard]] const BlockTypeData& blockTypeData() const { return getBlockTypeData(type_); }

    [[nodiscard]] Rectangle textureRectTop() const {
        return Rectangle{
            static_cast<float>(blockTypeData().textureAtlasPositionTop.x) * TEXTURE_SIZE,
            static_cast<float>(blockTypeData().textureAtlasPositionTop.y) * TEXTURE_SIZE,
            TEXTURE_SIZE, TEXTURE_SIZE};
    }
    [[nodiscard]] Rectangle textureRectBottom() const {
        return Rectangle{
            static_cast<float>(blockTypeData().textureAtlasPositionBottom.x) * TEXTURE_SIZE,
            static_cast<float>(blockTypeData().textureAtlasPositionBottom.y) * TEXTURE_SIZE,
            TEXTURE_SIZE, TEXTURE_SIZE};
    }
    [[nodiscard]] Rectangle textureRectSides() const {
        return Rectangle{
            static_cast<float>(blockTypeData().textureAtlasPositionSides.x) * TEXTURE_SIZE,
            static_cast<float>(blockTypeData().textureAtlasPositionSides.y) * TEXTURE_SIZE,
            TEXTURE_SIZE, TEXTURE_SIZE};
    }
};
