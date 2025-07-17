#pragma once

#include <array>

#include "raylib.h"

enum BlockType { BLOCK_AIR, BLOCK_DIRT, BLOCK_STONE };

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
        // unload GPU buffers and CPU-side mesh data:
        UnloadModel(cubeModel);
    }

    void init();
    void run();

   private:
    constexpr static int mapWidth = 128;
    constexpr static int mapDepth = 128;
    constexpr static int mapHeight = 16;

    constexpr static int seed = 1;  // Seed for noise generation

    typedef std::array<std::array<std::array<BlockType, mapHeight>, mapDepth>, mapWidth> World;

    Camera camera{};
    World world{};

    const Model cubeModel = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));

    void draw() const;
    void drawCursor() const;
    void drawFps() const;
};