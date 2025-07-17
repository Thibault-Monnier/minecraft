#include "game.hpp"
#include "menu.hpp"
#include "raylib.h"

int main() {
    InitWindow(0, 0, "Minecraft Clone");
    ToggleFullscreen();

    SetTargetFPS(60);  // Set the game to run at 60 frames per second

    runMenu();

    Game game;
    game.init();
    game.run();

    CloseWindow();  // Close the window and OpenGL context
}