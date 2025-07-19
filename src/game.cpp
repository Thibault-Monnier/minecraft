#include "game.hpp"

#include <array>
#include <cstdio>
#include <format>
#include <vector>

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
    std::vector<Matrix> grassTransforms;
    std::vector<Matrix> dirtTransforms;
    std::vector<Matrix> stoneTransforms;

    for (int x = 0; x < mapWidth; ++x) {
        for (int z = 0; z < mapDepth; ++z) {
            for (int y = 0; y < mapHeight; ++y) {
                const Vector3 pos = {static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f,
                                     static_cast<float>(z) + 0.5f};
                Matrix model = MatrixTranslate(pos.x, pos.y, pos.z);
                if (world[x][z][y] == BlockType::BLOCK_GRASS) {
                    grassTransforms.push_back(model);
                } else if (world[x][z][y] == BlockType::BLOCK_DIRT) {
                    dirtTransforms.push_back(model);
                } else if (world[x][z][y] == BlockType::BLOCK_STONE) {
                    stoneTransforms.push_back(model);
                } else if (world[x][z][y] == BlockType::BLOCK_AIR) {
                    // Do nothing for air blocks
                } else {
                    throw std::runtime_error(
                        std::format("Unknown block type at ({}, {}, {}): got {}", x, y, z,
                                    static_cast<int>(world[x][z][y])));
                }
            }
        }
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(camera);

    if (!grassTransforms.empty())
        DrawMeshInstanced(cubeMesh, materialGrass, grassTransforms.data(),
                          static_cast<int>(grassTransforms.size()));
    if (!dirtTransforms.empty())
        DrawMeshInstanced(cubeMesh, materialDirt, dirtTransforms.data(),
                          static_cast<int>(dirtTransforms.size()));
    if (!stoneTransforms.empty())
        DrawMeshInstanced(cubeMesh, materialStone, stoneTransforms.data(),
                          static_cast<int>(stoneTransforms.size()));

    DrawGrid(1000, 1.0f);  // Draw a grid for reference

    EndMode3D();

    drawCursor();
    drawFps();

    EndDrawing();
}

void Game::init() {
    DisableCursor();

    instancedShader = LoadShader(
        std::format("{}/resources/shaders/lighting_instancing.vs", CMAKE_ROOT_DIR).c_str(),
        nullptr);

    cubeMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    UploadMesh(&cubeMesh, false);

    Texture2D grassTexture =
        LoadTexture(std::format("{}/resources/textures/grass.png", CMAKE_ROOT_DIR).c_str());
    materialGrass = LoadMaterialDefault();
    materialGrass.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    materialGrass.maps[MATERIAL_MAP_DIFFUSE].texture = grassTexture;
    materialGrass.shader = instancedShader;

    Texture2D dirtTexture =
        LoadTexture(std::format("{}/resources/textures/dirt.png", CMAKE_ROOT_DIR).c_str());
    materialDirt = LoadMaterialDefault();
    materialDirt.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    materialDirt.maps[MATERIAL_MAP_DIFFUSE].texture = dirtTexture;  // Using the same texture
    materialDirt.shader = instancedShader;

    Texture2D stoneTexture =
        LoadTexture(std::format("{}/resources/textures/stone.png", CMAKE_ROOT_DIR).c_str());
    materialStone = LoadMaterialDefault();
    materialStone.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    materialStone.maps[MATERIAL_MAP_DIFFUSE].texture = stoneTexture;
    materialStone.shader = instancedShader;

    const float noiseScale = 0.01f;  // controls noise “zoom”

    for (int x = 0; x < mapWidth; x++) {
        for (int z = 0; z < mapDepth; z++) {
            float height =
                stb_perlin_noise3_seed(static_cast<float>(x) * noiseScale,
                                       static_cast<float>(z) * noiseScale, 0.0f, 0, 0, 0, seed);
            height = (height + 1.0f) / 2.0f;     // Normalize to (0, 1)
            height = height * mapHeight;         // Scale height
            int realHeight = std::ceil(height);  // Round to next integer in (0, mapHeight]

            for (int y = 0; y < mapHeight; ++y) {
                if (y < realHeight) {
                    if (y < 3)
                        world[x][z][y] = BlockType::BLOCK_STONE;  // Stone for lower layers
                    else if (y < realHeight - 1)
                        world[x][z][y] = BlockType::BLOCK_DIRT;  // Grass for upper layers
                    else
                        world[x][z][y] = BlockType::BLOCK_GRASS;  // Grass on top
                } else {
                    world[x][z][y] = BlockType::BLOCK_AIR;  // Air above the terrain
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