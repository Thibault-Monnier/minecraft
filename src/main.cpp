#include "Game.hpp"
#include "Menu.hpp"
#include "raylib.h"

int main() {
    InitWindow(0, 0, "Minecraft Clone");
    ToggleFullscreen();

    SetTargetFPS(60);

    runMenu();

    if (!WindowShouldClose()) {
        Game game;
        game.init();
        game.run();
    }

    CloseWindow();  // Close the window and OpenGL context
}