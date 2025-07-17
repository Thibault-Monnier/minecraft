#include "raylib.h"
#include "raymath.h"

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

void drawCursor() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    Vector2 center = {static_cast<float>(screenWidth) / 2.0f,
                      static_cast<float>(screenHeight) / 2.0f};

    DrawCircleLinesV(center, 7.0f, BLACK);  // Draw a circle around the cursor
    DrawCircleLinesV(center, 1.0f, BLACK);  // Draw a dot in the center
}

void drawGame(const Camera& camera) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(camera);

    Vector3 cubePosition = {0.5f, 0.5f, 0.5f};
    DrawCube(cubePosition, 1.0f, 1.0f, 1.0f, BROWN);
    DrawCubeWires(cubePosition, 1.0f, 1.0f, 1.0f, DARKGRAY);
    DrawGrid(1000, 1.0f);  // Draw a grid for reference

    EndMode3D();

    drawCursor();

    EndDrawing();
}

void runGame() {
    DisableCursor();

    Camera camera;
    camera.position = {-4.0f, 4.0f, 0.0f};
    camera.target = {0.0f, 0.0f, 0.0f};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    const float cameraMovementSpeed = 2.0f;   // Speed of camera movement
    const float cameraSensitivity = 0.0025f;  // Sensitivity for mouse movement

    float cameraYaw = 0.0f;    // Yaw angle for camera rotation
    float cameraPitch = 0.0f;  // Pitch angle for camera rotation

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
        else if (IsKeyDown(KEY_LEFT_SHIFT))
            moveUp = -1.0f;

        const float deltaTime = GetFrameTime();
        const float speedFactor = deltaTime * cameraMovementSpeed;

        Vector2 forward2D = {cosf(cameraYaw), sinf(cameraYaw)};
        Vector2 right2D = {forward2D.y, -forward2D.x};

        camera.position.x +=
            (forward2D.x * movement2DRelative.x + right2D.x * movement2DRelative.y) * speedFactor;
        camera.position.z +=
            (forward2D.y * movement2DRelative.x + right2D.y * movement2DRelative.y) * speedFactor;

        camera.position.y += moveUp * speedFactor;

        // Update camera target
        // This is done after updating the position to prevent clipping when it moves
        camera.target = Vector3Add(camera.position, direction);

        // Render
        drawGame(camera);
    }
}

int main() {
    InitWindow(0, 0, "Minecraft Clone");
    ToggleFullscreen();

    SetTargetFPS(60);  // Set the game to run at 60 frames per second

    runMenu();
    runGame();

    CloseWindow();  // Close the window and OpenGL context
}