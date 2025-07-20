#include "game.hpp"

#include <cstdio>
#include <format>
#include <iostream>
#include <ranges>
#include <vector>

#define RLIGHTS_IMPLEMENTATION
#include "raylib.h"
#include "raymath.h"
#include "rlights.h"
#include "utilityStructures.hpp"

bool Game::isPositionInRenderDistance(const Vector3& position) const {
    constexpr float maxDistanceSq =
        RENDER_DISTANCE * RENDER_DISTANCE * Chunk::CHUNK_SIZE * Chunk::CHUNK_SIZE;
    return Vector3DistanceSqr(position, camera_.position) < maxDistanceSq;
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
    SetShaderValue(terrainShader_, terrainShader_.locs[SHADER_LOC_VECTOR_VIEW], &camera_.position,
                   SHADER_UNIFORM_VEC3);

    BeginDrawing();
    ClearBackground(RAYWHITE);

    drawSky();

    BeginMode3D(camera_);

    // const auto startTime = static_cast<float>(GetTime());
    for (const auto& chunk : world_ | std::views::values) {
        if (isPositionInRenderDistance(chunk.getCenterPosition())) chunk.render();
    }
    // const auto endTime = static_cast<float>(GetTime());

    EndMode3D();

    drawCursor();
    drawFps();
    drawPositionInfo(camera_.position);

    EndDrawing();

    // std::cout << "Chunks rendering time: " << (endTime - startTime) * 1000.0f << " ms" <<
    // std::endl;
}

void Game::init() {
    DisableCursor();
    SetTargetFPS(0);  // Set to maximum FPS

    terrainShader_ = LoadShader(
        std::format("{}/resources/shaders/lighting_instancing.vs", CMAKE_ROOT_DIR).c_str(),
        std::format("{}/resources/shaders/lighting.fs", CMAKE_ROOT_DIR).c_str());

    float ambient[4] = {10.0f, 10.0f, 10.0f, 1.0f};
    int ambLoc = GetShaderLocation(terrainShader_, "ambient");
    SetShaderValue(terrainShader_, ambLoc, ambient, SHADER_UNIFORM_VEC4);

    constexpr Vector3 lightPos = {5000.0f, 15000.0f, 7500.0f};
    constexpr Color lightColor = {200, 200, 200, 255};
    UpdateLightValues(terrainShader_, CreateLight(LIGHT_DIRECTIONAL, lightPos, Vector3Zero(),
                                                  lightColor, terrainShader_));

    cubeMesh_ = GenMeshCube(1.0f, 1.0f, 1.0f);
    UploadMesh(&cubeMesh_, false);

    const Texture2D grassTexture =
        LoadTexture(std::format("{}/resources/textures/grass.png", CMAKE_ROOT_DIR).c_str());
    materialGrass_ = LoadMaterialDefault();
    materialGrass_.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    materialGrass_.maps[MATERIAL_MAP_DIFFUSE].texture = grassTexture;
    materialGrass_.shader = terrainShader_;

    const Texture2D dirtTexture =
        LoadTexture(std::format("{}/resources/textures/dirt.png", CMAKE_ROOT_DIR).c_str());
    materialDirt_ = LoadMaterialDefault();
    materialDirt_.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    materialDirt_.maps[MATERIAL_MAP_DIFFUSE].texture = dirtTexture;  // Using the same texture
    materialDirt_.shader = terrainShader_;

    const Texture2D stoneTexture =
        LoadTexture(std::format("{}/resources/textures/stone.png", CMAKE_ROOT_DIR).c_str());
    materialStone_ = LoadMaterialDefault();
    materialStone_.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    materialStone_.maps[MATERIAL_MAP_DIFFUSE].texture = stoneTexture;
    materialStone_.shader = terrainShader_;

    constexpr double chunksUpperBound =
        static_cast<float>((RENDER_DISTANCE + M_SQRT1_2) * (RENDER_DISTANCE + M_SQRT1_2)) * M_PI *
        MAP_HEIGHT_BLOCKS / Chunk::CHUNK_SIZE;
    world_.reserve(static_cast<size_t>(chunksUpperBound));

    double first = 0.0;
    double second = 0.0;

    auto generateChunk = [&](const Vector3Int& pos) {
        double time = GetTime();
        auto [it, _] = world_.emplace(pos, Chunk(pos.x, pos.y, pos.z, terrainShader_, cubeMesh_,
                                                 materialGrass_, materialDirt_, materialStone_));
        double time2 = GetTime();
        it->second.generate(SEED, MAP_HEIGHT_BLOCKS);
        double time3 = GetTime();
        first += time2 - time;
        second += time3 - time2;
    };

    // Generate spawn chunks first to know the starting position for accurate render distance
    double startTime = GetTime();
    for (int y = 0; y < MAP_HEIGHT_BLOCKS / Chunk::CHUNK_SIZE; y++) {
        generateChunk({0, y, 0});
    }
    double intermediateTime = GetTime();

    // Determine the starting position
    int startY = 0;
    while (
        world_.at({0, startY / Chunk::CHUNK_SIZE, 0}).getData()[0][startY % Chunk::CHUNK_SIZE][0] !=
        BlockType::BLOCK_AIR) {
        startY++;
    }
    startY += 2;                                                  // Start above the ground
    camera_.position = {0.5f, static_cast<float>(startY), 0.5f};  // Middle of the block

    // Generate the remaining chunks
    for (int x = -RENDER_DISTANCE; x < RENDER_DISTANCE; x++) {
        for (int y = 0; y < MAP_HEIGHT_BLOCKS / Chunk::CHUNK_SIZE; y++) {
            for (int z = -RENDER_DISTANCE; z < RENDER_DISTANCE; z++) {
                if (x == 0 && z == 0) continue;

                if (!isPositionInRenderDistance(
                        Vector3{static_cast<float>(x * Chunk::CHUNK_SIZE) + 0.5f,
                                static_cast<float>(y * Chunk::CHUNK_SIZE) + 0.5f,
                                static_cast<float>(z * Chunk::CHUNK_SIZE) +
                                    0.5f})) {  // Center of the chunk
                    continue;
                }

                generateChunk({x, y, z});
            }
        }
    }
    double endTime = GetTime();
    std::cout << "Chunks generation time: " << (endTime - startTime) * 1000.0f << " ms" << std::endl
              << "Chunks generated: " << world_.size() << std::endl;
    std::cout << "Intermediate time: " << (intermediateTime - startTime) * 1000.0f << " ms"
              << std::endl;

    std::cout << "Partial times: " << (first) * 1000.0f << " ms (chunk init), "
              << (second) * 1000.0f << " ms (chunk generation)" << std::endl;

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

        if (IsKeyDown(KEY_C)) {
            camera_.fovy = 20;  // Zoom in
        } else {
            camera_.fovy = 80;  // Normal
        }

        // Render
        draw();
    }
}