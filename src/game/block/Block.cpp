#include "Block.hpp"

#include <array>

void Block::generateBlockMesh(const Vector3Int& position, std::vector<Vertex>& chunkMeshVerts,
                              std::vector<uint16_t>& chunkMeshIndices_,
                              const std::array<bool, 6>& isFaceVisible,
                              const Texture2D& textureAtlas) const {
    if (!isRendered()) return;

    struct Face {
        Vector3Int originOffset;  // corner where u=0,v=0
        Vector3Int u, v;          // edge vectors
        Vector3Int normal;
    };

    static constexpr Face faces[6] = {
        // +X (east)
        {{1, 1, 0}, {0, 0, 1}, {0, -1, 0}, {1, 0, 0}},
        // -X (west)
        {{0, 1, 1}, {0, 0, -1}, {0, -1, 0}, {-1, 0, 0}},

        // +Y (top)
        {{0, 1, 0}, {0, 0, 1}, {1, 0, 0}, {0, 1, 0}},
        // -Y (bottom)
        {{0, 0, 0}, {1, 0, 0}, {0, 0, 1}, {0, -1, 0}},

        // +Z (back)
        {{1, 1, 1}, {-1, 0, 0}, {0, -1, 0}, {0, 0, 1}},
        // -Z (front)
        {{0, 1, 0}, {1, 0, 0}, {0, -1, 0}, {0, 0, -1}},
    };

    auto appendQuad = [&](const Vector3Int& origin, const Vector3Int& edgeDirU,
                          const Vector3Int& edgeDirV, const Vector3Int& faceNormal) {
        const int startIndex = static_cast<int>(chunkMeshVerts.size());

        Rectangle textureRect;
        if (faceNormal == Vector3Int{0, 1, 0}) {
            textureRect = textureRectTop();
        } else if (faceNormal == Vector3Int{0, -1, 0}) {
            textureRect = textureRectBottom();
        } else [[likely]] {
            textureRect = textureRectSides();
        }

        const float u0 = textureRect.x / static_cast<float>(textureAtlas.width);
        const float v0 = textureRect.y / static_cast<float>(textureAtlas.height);
        const float u1 =
            (textureRect.x + textureRect.width) / static_cast<float>(textureAtlas.width);
        const float v1 =
            (textureRect.y + textureRect.height) / static_cast<float>(textureAtlas.height);

        const Vector2 textureCoordBL{u0, v0};
        const Vector2 textureCoordBR{u1, v0};
        const Vector2 textureCoordTR{u1, v1};
        const Vector2 textureCoordTL{u0, v1};

        chunkMeshVerts.push_back({origin, faceNormal, textureCoordBL});
        chunkMeshVerts.push_back({origin + edgeDirU, faceNormal, textureCoordBR});
        chunkMeshVerts.push_back({origin + edgeDirU + edgeDirV, faceNormal, textureCoordTR});
        chunkMeshVerts.push_back({origin + edgeDirV, faceNormal, textureCoordTL});

        chunkMeshIndices_.push_back(startIndex + 0);
        chunkMeshIndices_.push_back(startIndex + 1);
        chunkMeshIndices_.push_back(startIndex + 2);
        chunkMeshIndices_.push_back(startIndex + 0);
        chunkMeshIndices_.push_back(startIndex + 2);
        chunkMeshIndices_.push_back(startIndex + 3);
    };

    for (int i = 0; i < 6; ++i) {
        if (!isFaceVisible[i]) continue;

        const Face& face = faces[i];
        const Vector3Int origin = {position.x + face.originOffset.x,
                                   position.y + face.originOffset.y,
                                   position.z + face.originOffset.z};
        appendQuad(origin, face.u, face.v, face.normal);
    }
}
