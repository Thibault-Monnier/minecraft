#include "menu.hpp"

#include "raylib.h"

void runMenu() {
    const int halfScreenWidth = GetScreenWidth() / 2;
    const int halfScreenHeight = GetScreenHeight() / 2;

    const char* title = "Welcome to the Minecraft Clone!";
    const char* subtitle = "Press ENTER to launch the game";

    const int titleFontSize = 60;
    const int subtitleFontSize = 30;

    const int titleHalfWidth = MeasureText(title, titleFontSize) / 2;
    const int titleHeight = titleFontSize;
    const int subtitleHalfWidth = MeasureText(subtitle, subtitleFontSize) / 2;
    const int subtitleHeight = subtitleFontSize;

    while (!(WindowShouldClose() || IsKeyPressed(KEY_ENTER))) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText(title, halfScreenWidth - titleHalfWidth, halfScreenHeight - titleHeight,
                 titleFontSize, ORANGE);
        DrawText(subtitle, halfScreenWidth - subtitleHalfWidth, halfScreenHeight + subtitleHeight,
                 subtitleFontSize, DARKGRAY);

        EndDrawing();
    }
}