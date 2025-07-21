#pragma once

#include "Chunk.hpp"
#include "Player.hpp"
#include "UtilityStructures.hpp"
#include "absl/container/flat_hash_map.h"
#include "raylib.h"

class Game {
   public:
    Game() = default;
    ~Game() {
        UnloadShader(terrainShader_);
        UnloadMesh(cubeMesh_);
    }

    void init();
    void run();

    constexpr static int RENDER_DISTANCE = 5;  // Render distance in chunks
    constexpr static int MAP_HEIGHT_BLOCKS = 512;

   private:
    constexpr static int SEED = 1;  // Seed for noise generation

    Player player_{};

    absl::flat_hash_map<Vector3Int, Chunk> world_{};

    Shader terrainShader_{};
    Mesh cubeMesh_{};
    Material materialGrass_{};
    Material materialDirt_{};
    Material materialStone_{};

    void draw() const;
    static void drawSky() ;
    static void drawCursor() ;
    static void drawFps() ;
    static void drawPositionInfo(const Vector3& position) ;
    [[nodiscard]] bool isPositionInRenderDistance(const Vector3& position) const;
};