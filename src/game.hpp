#pragma once

#include "absl/container/flat_hash_map.h"
#include "chunk.hpp"
#include "raylib.h"
#include "utilityStructures.hpp"

class Game {
   public:
    Game() {
        camera_.position = {0.0f, 0.0f, 0.0f};
        camera_.target = {0.0f, 0.0f, 0.0f};
        camera_.up = {0.0f, 1.0f, 0.0f};
        camera_.fovy = 80.0f;
        camera_.projection = CAMERA_PERSPECTIVE;
    }

    ~Game() {
        UnloadShader(terrainShader_);
        UnloadMesh(cubeMesh_);
    }

    void init();
    void run();

    constexpr static int RENDER_DISTANCE = 25;  // Render distance in chunks
    constexpr static int MAP_HEIGHT_BLOCKS = 512;

   private:
    constexpr static int SEED = 1;  // Seed for noise generation

    Camera camera_{};

    absl::flat_hash_map<Vector3Int, Chunk> world_{};

    Shader terrainShader_{};
    Mesh cubeMesh_{};
    Material materialGrass_{};
    Material materialDirt_{};
    Material materialStone_{};

    void draw() const;
    void drawSky() const;
    void drawCursor() const;
    void drawFps() const;
    void drawPositionInfo(const Vector3& position) const;
    bool isPositionInRenderDistance(const Vector3& position) const;
};