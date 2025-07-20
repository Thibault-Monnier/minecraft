#include "game.hpp"

#include <cstdio>
#include <format>
#include <iostream>
#include <ranges>
#include <vector>

#include "raylib.h"
#include "raymath.h"
#include "utilityStructures.hpp"

bool Game::isPositionInRenderDistance(const Vector3& position) const {
    return Vector3DistanceSqr(position, camera_.position) <
           RENDER_DISTANCE * RENDER_DISTANCE * Chunk::CHUNK_SIZE * Chunk::CHUNK_SIZE;
}

void Game::drawSky() const {
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();

    DrawRectangleGradientV(0, 0, screenWidth, screenHeight, {135, 206, 235, 255},
                           {65, 105, 225, 255});
}

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

void Game::drawPositionInfo(const Vector3& position) const {
    DrawRectangle(10, 10, 200, 80, Fade(BLACK, 0.35f));  // Semi-transparent background
    DrawRectangleLines(10, 10, 200, 80, BLACK);          // Border around the rectangle
    DrawText(TextFormat("X: %.2f\nY: %.2f\nZ: %.2f", position.x, position.y, position.z), 20, 20,
             20, BLACK);
}

void Game::draw() const {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    drawSky();

    BeginMode3D(camera_);

    const auto startTime = static_cast<float>(GetTime());
    for (const auto& chunk : world_ | std::views::values) {
        if (isPositionInRenderDistance(chunk.getPositionBlocks())) chunk.render();
    }
    const auto endTime = static_cast<float>(GetTime());

    EndMode3D();

    drawCursor();
    drawFps();
    drawPositionInfo(camera_.position);

    EndDrawing();

    std::cout << "Chunks rendering time: " << (endTime - startTime) * 1000.0f << " ms" << std::endl;
}

void Game::init() {
    DisableCursor();
    SetTargetFPS(0);  // Set to maximum FPS

    instancedShader_ = LoadShader(
        std::format("{}/resources/shaders/lighting_instancing.vs", CMAKE_ROOT_DIR).c_str(),
        nullptr);

    cubeMesh_ = GenMeshCube(1.0f, 1.0f, 1.0f);
    UploadMesh(&cubeMesh_, false);

    const Texture2D grassTexture =
        LoadTexture(std::format("{}/resources/textures/grass.png", CMAKE_ROOT_DIR).c_str());
    materialGrass_ = LoadMaterialDefault();
    materialGrass_.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    materialGrass_.maps[MATERIAL_MAP_DIFFUSE].texture = grassTexture;
    materialGrass_.shader = instancedShader_;

    const Texture2D dirtTexture =
        LoadTexture(std::format("{}/resources/textures/dirt.png", CMAKE_ROOT_DIR).c_str());
    materialDirt_ = LoadMaterialDefault();
    materialDirt_.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    materialDirt_.maps[MATERIAL_MAP_DIFFUSE].texture = dirtTexture;  // Using the same texture
    materialDirt_.shader = instancedShader_;

    const Texture2D stoneTexture =
        LoadTexture(std::format("{}/resources/textures/stone.png", CMAKE_ROOT_DIR).c_str());
    materialStone_ = LoadMaterialDefault();
    materialStone_.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    materialStone_.maps[MATERIAL_MAP_DIFFUSE].texture = stoneTexture;
    materialStone_.shader = instancedShader_;

    for (int x = -RENDER_DISTANCE; x < RENDER_DISTANCE; x++) {
        for (int y = 0; y < MAP_HEIGHT_BLOCKS / Chunk::CHUNK_SIZE; y++) {
            for (int z = -RENDER_DISTANCE; z < RENDER_DISTANCE; z++) {
                if (!isPositionInRenderDistance(
                        Vector3{static_cast<float>(x * Chunk::CHUNK_SIZE),
                                static_cast<float>(y * Chunk::CHUNK_SIZE),
                                static_cast<float>(z * Chunk::CHUNK_SIZE)})) {
                    continue;
                }

                auto [it, _] = world_.emplace(
                    Vector3Int{x, y, z}, Chunk(x, y, z, instancedShader_, cubeMesh_, materialGrass_,
                                               materialDirt_, materialStone_));
                it->second.generate(SEED, MAP_HEIGHT_BLOCKS);
            }
        }
    }

    for (auto& chunk : world_ | std::views::values) {
        const int x = chunk.getX();
        const int y = chunk.getY();
        const int z = chunk.getZ();

        auto findNeighbor = [&](const int dx, const int dy, const int dz) -> const Chunk* {
            const auto it = world_.find(Vector3Int{x + dx, y + dy, z + dz});
            return (it != world_.end()) ? &it->second : nullptr;
        };

        const Chunk* posX = findNeighbor(+1, 0, 0);
        const Chunk* negX = findNeighbor(-1, 0, 0);
        const Chunk* posY = findNeighbor(0, +1, 0);
        const Chunk* negY = findNeighbor(0, -1, 0);
        const Chunk* posZ = findNeighbor(0, 0, +1);
        const Chunk* negZ = findNeighbor(0, 0, -1);

        chunk.generateTransforms(posX, negX, posY, negY, posZ, negZ);
    }

    std::cout << "Generated " << world_.size() << " chunks." << std::endl;
}

void Game::run() {
    const float cameraMovementSpeedSlow = 4.3f;   // Speed of camera movement without SHIFT
    const float cameraMovementSpeedFast = 17.5f;  // Speed of camera movement when holding SHIFT
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

        camera_.position.x +=
            (forward2D.x * movement2DRelative.x + right2D.x * movement2DRelative.y) * speedFactor;
        camera_.position.z +=
            (forward2D.y * movement2DRelative.x + right2D.y * movement2DRelative.y) * speedFactor;

        camera_.position.y += moveUp * speedFactor * cameraMovementSpeedVerticalMultiplier;

        // Update camera target
        // This is done after updating the position to prevent clipping when it moves
        camera_.target = Vector3Add(camera_.position, direction);

        // Render
        draw();
    }
}