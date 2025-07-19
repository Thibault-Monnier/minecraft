#pragma once

#include <array>

#include "raylib.h"

enum class BlockType { BLOCK_AIR, BLOCK_GRASS, BLOCK_DIRT, BLOCK_STONE };

class Game {
   public:
    Game() {
        camera.position = {-4.0f, 4.0f, 0.0f};
        camera.target = {0.0f, 0.0f, 0.0f};
        camera.up = {0.0f, 1.0f, 0.0f};
        camera.fovy = 80.0f;
        camera.projection = CAMERA_PERSPECTIVE;
    }

    ~Game() {
        UnloadShader(instancedShader);
        UnloadMesh(cubeMesh);
        UnloadMaterial(materialGrass);
        UnloadMaterial(materialStone);
    }

    void init();
    void run();

   private:
    constexpr static int mapWidth = 216;
    constexpr static int mapDepth = 216;
    constexpr static int mapHeight = 32;

    constexpr static int seed = 1;  // Seed for noise generation

    typedef std::array<std::array<std::array<BlockType, mapDepth>, mapHeight>, mapWidth> World;

    Camera camera{};
    World world{};

    Mesh cubeMesh{};

    Material materialGrass{};
    Material materialDirt{};
    Material materialStone{};
    Shader instancedShader{};

    void draw() const;
    void drawCursor() const;
    void drawFps() const;
};