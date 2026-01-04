#include "game/Game.hpp"
#include "menu/Menu.hpp"
#include "raylib.h"

int main() {
    InitWindow(720, 480, "Minecraft Clone");
    if (!IsWindowFullscreen()) ToggleFullscreen();

    SetTargetFPS(60);

    runMenu();

    if (!WindowShouldClose()) {
        Game game;
        game.init();
        game.run();
    }

    CloseWindow();  // Close the window and OpenGL context
}