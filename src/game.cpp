#include "game.hpp"

#include <array>
#include <cstdio>

#include "GL/gl.h"
#include "raylib.h"
#include "raymath.h"
#include "stb_perlin.h"

void Game::drawCursor() const {
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();
    const Vector2 center = {static_cast<float>(screenWidth) / 2.0f,
                            static_cast<float>(screenHeight) / 2.0f};

    DrawCircleLinesV(center, 7.0f, BLACK);  // Draw a circle around the cursor
    DrawCircleLinesV(center, 1.0f, BLACK);  // Draw a dot in the center
}

void Game::drawFps() const {
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();
    DrawText(TextFormat("FPS: %i", GetFPS()), screenWidth - 100, screenHeight - 30, 20, BLACK);
}

void Game::draw() const {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(camera);

    for (int x = 0; x < mapWidth; ++x) {
        for (int z = 0; z < mapDepth; ++z) {
            for (int y = 0; y < mapHeight; ++y) {
                const Vector3 position = {static_cast<float>(x) + 0.5f,
                                          static_cast<float>(y) + 0.5f,
                                          static_cast<float>(z) + 0.5f};
                const Vector3 size = {1.0f, 1.0f, 1.0f};

                if (world[x][z][y] == BLOCK_DIRT) {
                    DrawCubeV(position, size, BROWN);
                    DrawCubeWiresV(position, size, DARKBROWN);
                } else if (world[x][z][y] == BLOCK_STONE) {
                    DrawCubeV(position, size, GRAY);
                    DrawCubeWiresV(position, size, DARKGRAY);
                }
            }
        }
    }

    DrawGrid(1000, 1.0f);  // Draw a grid for reference

    EndMode3D();

    drawCursor();
    drawFps();  // Draw FPS counter

    EndDrawing();
}

void Game::init(int a) {
    DisableCursor();

    const float noiseScale = 0.01f;  // controls noise “zoom”

    for (int x = 0; x < mapWidth; x++) {
        for (int z = 0; z < mapDepth; z++) {
            float height =
                stb_perlin_noise3_seed(static_cast<float>(x) * noiseScale,
                                       static_cast<float>(z) * noiseScale, 0.0f, 0, 0, 0, seed);
            height = (height + 1.0f) / 2.0f;     // Normalize to (0, 1)
            height = height * mapHeight;         // Scale height
            int realHeight = std::ceil(height);  // Round down to next integer in (0, mapHeight]

            for (int y = 0; y < 10; ++y) {
                if (y < realHeight) {
                    if (y < 3)
                        world[x][z][y] = BLOCK_STONE;  // Stone for lower layers
                    else
                        world[x][z][y] = BLOCK_DIRT;  // Dirt for upper layers
                } else {
                    world[x][z][y] = BLOCK_AIR;  // Air above the terrain
                }
            }
        }
    }
}

void Game::run() {
    const float cameraMovementSpeedSlow = 4.3f;  // Speed of camera movement without SHIFT
    const float cameraMovementSpeedFast = 9.5f;  // Speed of camera movement when holding SHIFT
    const float cameraMovementSpeedVerticalMultiplier = 1.5f;  // Vertical movement speed multiplier
    const float cameraSensitivity = 0.0025f;                   // Sensitivity for mouse movement

    float cameraYaw = 0.0f;    // Yaw angle for camera rotation
    float cameraPitch = 0.0f;  // Pitch angle for camera rotation

    float cameraCurrentMovementSpeed = cameraMovementSpeedSlow;  // Current speed of the camera

    while (!WindowShouldClose()) {
        // Calculate camera rotation
        Vector2 mouseDelta = GetMouseDelta();
        cameraYaw += mouseDelta.x * cameraSensitivity;
        cameraPitch -= mouseDelta.y * cameraSensitivity;
        cameraPitch = Clamp(cameraPitch, -89.0f / 180.0f * M_PI,
                            89.0f / 180.0f * M_PI);  // Limit pitch to avoid flipping

        Vector3 direction = {cosf(cameraYaw) * cosf(cameraPitch), sinf(cameraPitch),
                             sinf(cameraYaw) * cosf(cameraPitch)};

        // Update camera position
        Vector2 movement2DRelative = {0.0f, 0.0f};
        float moveUp = 0.0f;

        if (IsKeyDown(KEY_W))
            movement2DRelative.x = 1.0f;
        else if (IsKeyDown(KEY_S))
            movement2DRelative.x = -1.0f;

        if (IsKeyDown(KEY_A))
            movement2DRelative.y = 1.0f;
        else if (IsKeyDown(KEY_D))
            movement2DRelative.y = -1.0f;

        movement2DRelative = Vector2Normalize(movement2DRelative);

        if (IsKeyDown(KEY_SPACE))
            moveUp = 1.0f;
        else if (IsKeyDown(KEY_LEFT_CONTROL))
            moveUp = -1.0f;

        if (IsKeyPressed(KEY_LEFT_SHIFT)) {
            cameraCurrentMovementSpeed = (cameraCurrentMovementSpeed == cameraMovementSpeedSlow)
                                             ? cameraMovementSpeedFast
                                             : cameraMovementSpeedSlow;
        }

        const float deltaTime = GetFrameTime();
        const float speedFactor = deltaTime * cameraCurrentMovementSpeed;

        Vector2 forward2D = {cosf(cameraYaw), sinf(cameraYaw)};
        Vector2 right2D = {forward2D.y, -forward2D.x};

        camera.position.x +=
            (forward2D.x * movement2DRelative.x + right2D.x * movement2DRelative.y) * speedFactor;
        camera.position.z +=
            (forward2D.y * movement2DRelative.x + right2D.y * movement2DRelative.y) * speedFactor;

        camera.position.y += moveUp * speedFactor * cameraMovementSpeedVerticalMultiplier;

        // Update camera target
        // This is done after updating the position to prevent clipping when it moves
        camera.target = Vector3Add(camera.position, direction);

        // Render
        draw();
    }
}