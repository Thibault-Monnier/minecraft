#pragma once

#include <array>
#include <vector>

#include "chunk.hpp"
#include "raylib.h"

class Game {
   public:
    Game() {
        camera_.position = {-4.0f, 4.0f, 0.0f};
        camera_.target = {0.0f, 0.0f, 0.0f};
        camera_.up = {0.0f, 1.0f, 0.0f};
        camera_.fovy = 80.0f;
        camera_.projection = CAMERA_PERSPECTIVE;
    }

    ~Game() {
        UnloadShader(instancedShader_);
        UnloadMesh(cubeMesh_);
        UnloadMaterial(materialGrass_);
        UnloadMaterial(materialStone_);
    }

    void init();
    void run();

    constexpr static int MAP_WIDTH_CHUNKS = 16;
    constexpr static int MAP_DEPTH_CHUNKS = 16;
    constexpr static int MAP_HEIGHT_CHUNKS = 2;

   private:
    constexpr static int SEED = 1;  // Seed for noise generation

    Camera camera_{};
    std::vector<Chunk> world_{};

    Shader instancedShader_{};
    Mesh cubeMesh_{};
    Material materialGrass_{};
    Material materialDirt_{};
    Material materialStone_{};

    void draw() const;
    void drawCursor() const;
    void drawFps() const;
};