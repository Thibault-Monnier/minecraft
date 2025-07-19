#pragma once

#include <array>
#include <vector>

#include "raylib.h"

enum class BlockType { BLOCK_AIR, BLOCK_GRASS, BLOCK_DIRT, BLOCK_STONE };

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

    constexpr static int MAP_WIDTH = 216;
    constexpr static int MAP_DEPTH = 216;
    constexpr static int MAP_HEIGHT = 32;

    Mesh cubeMesh_{};

    Material materialGrass_{};
    Material materialDirt_{};
    Material materialStone_{};
    Shader instancedShader_{};

   private:
    constexpr static int SEED = 1;  // Seed for noise generation

    typedef std::array<std::array<std::array<BlockType, MAP_DEPTH>, MAP_HEIGHT>, MAP_WIDTH> World;

    Camera camera_{};
    World world_{};

    std::vector<Matrix> grassTransforms;
    std::vector<Matrix> dirtTransforms;
    std::vector<Matrix> stoneTransforms;

    void draw() const;
    void drawCursor() const;
    void drawFps() const;
};