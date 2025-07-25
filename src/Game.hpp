#pragma once

#include "Chunk.hpp"
#include "Player.hpp"
#include "UtilityStructures.hpp"
#include "absl/container/flat_hash_map.h"
#include "raylib.h"

class Game {
   public:
    Game() = default;
    ~Game() { UnloadMaterial(materialAtlas_); }

    void init();
    void run();

    constexpr static int RENDER_DISTANCE = 25;  // Render distance in chunks
    constexpr static int MAP_HEIGHT_BLOCKS = 512;

   private:
    constexpr static int SEED = 1;  // Seed for noise generation

    Player player_{};

    absl::flat_hash_map<Vector3Int, std::unique_ptr<Chunk>> world_{};

    Shader terrainShader_{};
    Material materialAtlas_{};

    [[nodiscard]] bool isPositionInRenderDistance(const Vector3& position) const;

    static void drawSky();
    static void drawCursor();
    static void drawFps();
    static void drawPositionInfo(const Vector3& position);
    void draw() const;

    [[nodiscard]] std::array<OptionalRef<Chunk>, 6> findAdjacentChunks(const Chunk& chunk) const;

    Chunk& generateChunk(const Vector3Int& pos);
    void generateChunkTransforms(Chunk& chunk) const;
    void updateTerrain();
};