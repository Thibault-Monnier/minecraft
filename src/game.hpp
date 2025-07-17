#pragma once

#include <array>

#include "raylib.h"

enum BlockType { BLOCK_AIR, BLOCK_DIRT, BLOCK_STONE };

typedef std::array<std::array<std::array<BlockType, 1>, 16>, 16> World;

class Game {
   public:
    Game() {
        camera.position = {-4.0f, 4.0f, 0.0f};
        camera.target = {0.0f, 0.0f, 0.0f};
        camera.up = {0.0f, 1.0f, 0.0f};
        camera.fovy = 80.0f;
        camera.projection = CAMERA_PERSPECTIVE;
    }

    void init();
    void run();

   private:
    Camera camera;
    World world = {};

    void draw() const;
    void drawCursor() const;
};