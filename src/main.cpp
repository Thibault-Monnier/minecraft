#include "game.hpp"
#include "menu.hpp"
#include "raylib.h"

int main() {
    InitWindow(0, 0, "Minecraft Clone");
    ToggleFullscreen();

    SetTargetFPS(1e4);

    runMenu();

    Game game;
    game.init();
    game.run();

    CloseWindow();  // Close the window and OpenGL context
}