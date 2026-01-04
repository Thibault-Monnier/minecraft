#pragma once

#include "Chunk.hpp"
#include "Player.hpp"
#include "absl/container/flat_hash_map.h"
#include "common/UtilityStructures.hpp"
#include "raylib.h"

class Game {
   public:
    Game() = default;
    ~Game() { UnloadMaterial(materialAtlas_); }

    void init();
    void run();

   private:
    constexpr static int DEFAULT_RENDER_DISTANCE = 15;  // Render distance in chunks
    constexpr static int MAP_HEIGHT_BLOCKS = 512;
    constexpr static int SEED = 1;  // Seed for noise generation

    int renderDistance_ = DEFAULT_RENDER_DISTANCE;

    Player player_{};

    absl::flat_hash_map<Vector3Int, std::unique_ptr<Chunk>> world_{};

    Shader terrainShader_{};
    Material materialAtlas_{};

    [[nodiscard]] bool isPositionInRenderDistance(const Vector3& position) const;

    static void drawSky();
    static void drawCursor();
    static void drawFps();
    void drawRenderDistance() const;
    static void drawPositionInfo(const Vector3& position);
    void draw() const;

    [[nodiscard]] std::array<OptionalRef<Chunk>, 6> findAdjacentChunks(const Chunk& chunk) const;

    Chunk& generateChunk(const Vector3Int& pos);
    void generateChunkTransforms(Chunk& chunk) const;

    /// If needed, updates chunk transforms and generates new chunks within the render distance
    /// around the player
    void updateTerrain();

    /// Update the shader used in the material atlas to the current terrain shader
    ///
    /// Note: chunk transforms must be re-generated separately after changing the shader
    void updateShader();
    void updateFog();
};