#include "game.hpp"

#include <array>

#include "raylib.h"
#include "raymath.h"

void Game::drawCursor() const {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    Vector2 center = {static_cast<float>(screenWidth) / 2.0f,
                      static_cast<float>(screenHeight) / 2.0f};

    DrawCircleLinesV(center, 7.0f, BLACK);  // Draw a circle around the cursor
    DrawCircleLinesV(center, 1.0f, BLACK);  // Draw a dot in the center
}

void Game::draw() const {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(camera);

    for (int x = 0; x < 16; ++x) {
        for (int z = 0; z < 16; ++z) {
            const Vector3 position = {static_cast<float>(x) + 0.5f, 0.5f,
                                      static_cast<float>(z) + 0.5f};
            const Vector3 size = {1.0f, 1.0f, 1.0f};

            if (world[x][z][0] == BLOCK_DIRT) {
                DrawCubeV(position, size, BROWN);
                DrawCubeWiresV(position, size, DARKBROWN);  // Draw wireframe for better visibility
            } else if (world[x][z][0] == BLOCK_STONE) {
                DrawCubeV(position, size, GRAY);
                DrawCubeWiresV(position, size, DARKGRAY);  // Draw wireframe for better visibility
            }
        }
    }

    DrawGrid(1000, 1.0f);  // Draw a grid for reference

    EndMode3D();

    drawCursor();

    EndDrawing();
}

void Game::init() {
    // Initialize the world
    for (int x = 0; x < 16; ++x) {
        for (int y = 0; y < 16; ++y) {
            if (x % 2 == 0)
                world[x][y][0] = BLOCK_DIRT;
            else
                world[x][y][0] = BLOCK_STONE;
        }
    }
    world[8][8][0] = BLOCK_AIR;
    world[7][7][0] = BLOCK_AIR;
    world[8][7][0] = BLOCK_AIR;
    world[7][8][0] = BLOCK_AIR;

    DisableCursor();
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