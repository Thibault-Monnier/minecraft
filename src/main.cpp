#include "game/Game.hpp"
#include "menu/Menu.hpp"
#include "raylib.h"

int main() {
    SetConfigFlags(FLAG_WINDOW_UNDECORATED);
    InitWindow(0, 0, "Minecraft Clone");

    SetTargetFPS(60);

    runMenu();

    if (!WindowShouldClose()) {
        Game game;
        game.init();
        game.run();
    }

    CloseWindow();  // Close the window and OpenGL context
}