#include "raylib.h"
#include "raymath.h"

void runMenu() {
    while (!(WindowShouldClose() || IsKeyPressed(KEY_ENTER))) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Welcome to the Minecraft Clone!", 190, 200, 30, ORANGE);
        DrawText("Press ENTER to launch the game", 220, 250, 20, DARKGRAY);

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
        float moveForward = 0.0f;
        float moveSide = 0.0f;
        float moveUp = 0.0f;

        if (IsKeyDown(KEY_W))
            moveForward = 1.0f;
        else if (IsKeyDown(KEY_S))
            moveForward = -1.0f;

        if (IsKeyDown(KEY_A))
            moveSide = 1.0f;
        else if (IsKeyDown(KEY_D))
            moveSide = -1.0f;

        if (IsKeyDown(KEY_SPACE))
            moveUp = 1.0f;
        else if (IsKeyDown(KEY_LEFT_SHIFT))
            moveUp = -1.0f;

        const float deltaTime = GetFrameTime();
        const float speedFactor = deltaTime * cameraMovementSpeed;

        Vector2 forward2D = {cosf(cameraYaw), sinf(cameraYaw)};
        Vector2 right2D = {forward2D.y, -forward2D.x};

        camera.position.x += (forward2D.x * moveForward + right2D.x * moveSide) * speedFactor;
        camera.position.z += (forward2D.y * moveForward + right2D.y * moveSide) * speedFactor;

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