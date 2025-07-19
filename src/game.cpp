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
    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(camera_);

    const float startTime = GetTime();

    if (!grassTransforms.empty())
        DrawMeshInstanced(cubeMesh_, materialGrass_, grassTransforms.data(),
                          static_cast<int>(grassTransforms.size()));
    if (!dirtTransforms.empty())
        DrawMeshInstanced(cubeMesh_, materialDirt_, dirtTransforms.data(),
                          static_cast<int>(dirtTransforms.size()));
    if (!stoneTransforms.empty())
        DrawMeshInstanced(cubeMesh_, materialStone_, stoneTransforms.data(),
                          static_cast<int>(stoneTransforms.size()));

    const float endTime = GetTime();

    DrawGrid(1000, 1.0f);  // Draw a grid for reference

    EndMode3D();

    drawCursor();
    drawFps();

    EndDrawing();

    std::printf(
        "Rendered %d blocks in %.4f\n",
        static_cast<int>(grassTransforms.size() + dirtTransforms.size() + stoneTransforms.size()),
        endTime - startTime);
}

void Game::init() {
    DisableCursor();

    instancedShader_ = LoadShader(
        std::format("{}/resources/shaders/lighting_instancing.vs", CMAKE_ROOT_DIR).c_str(),
        nullptr);

    cubeMesh_ = GenMeshCube(1.0f, 1.0f, 1.0f);
    UploadMesh(&cubeMesh_, false);

    Texture2D grassTexture =
        LoadTexture(std::format("{}/resources/textures/grass.png", CMAKE_ROOT_DIR).c_str());
    materialGrass_ = LoadMaterialDefault();
    materialGrass_.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    materialGrass_.maps[MATERIAL_MAP_DIFFUSE].texture = grassTexture;
    materialGrass_.shader = instancedShader_;

    Texture2D dirtTexture =
        LoadTexture(std::format("{}/resources/textures/dirt.png", CMAKE_ROOT_DIR).c_str());
    materialDirt_ = LoadMaterialDefault();
    materialDirt_.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    materialDirt_.maps[MATERIAL_MAP_DIFFUSE].texture = dirtTexture;  // Using the same texture
    materialDirt_.shader = instancedShader_;

    Texture2D stoneTexture =
        LoadTexture(std::format("{}/resources/textures/stone.png", CMAKE_ROOT_DIR).c_str());
    materialStone_ = LoadMaterialDefault();
    materialStone_.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    materialStone_.maps[MATERIAL_MAP_DIFFUSE].texture = stoneTexture;
    materialStone_.shader = instancedShader_;

    const float noiseScale = 0.01f;  // controls noise “zoom”

    for (int x = 0; x < MAP_WIDTH; x++) {
        for (int z = 0; z < MAP_DEPTH; z++) {
            float height =
                stb_perlin_noise3_seed(static_cast<float>(x) * noiseScale,
                                       static_cast<float>(z) * noiseScale, 0.0f, 0, 0, 0, SEED);
            height = (height + 1.0f) / 2.0f;     // Normalize to (0, 1)
            height = height * MAP_HEIGHT;        // Scale height
            int realHeight = std::ceil(height);  // Round to next integer in (0, mapHeight]

            for (int y = 0; y < MAP_HEIGHT; ++y) {
                if (y < realHeight) {
                    if (y < realHeight - 4)
                        world_[x][y][z] = BlockType::BLOCK_STONE;  // Stone for lower layers
                    else if (y < realHeight - 1)
                        world_[x][y][z] = BlockType::BLOCK_DIRT;  // Grass for upper layers
                    else
                        world_[x][y][z] = BlockType::BLOCK_GRASS;  // Grass on top
                } else {
                    world_[x][y][z] = BlockType::BLOCK_AIR;  // Air above the terrain
                }
            }
        }
    }

    const float startTime = GetTime();

    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int z = 0; z < MAP_DEPTH; ++z) {
            for (int y = 0; y < MAP_HEIGHT; ++y) {
                if (world_[x][y][z] == BlockType::BLOCK_AIR) {
                    continue;  // Skip air blocks
                }

                constexpr std::array<Vector3, 6> neighbourCubesOffsets = {
                    {Vector3{-1.0f, 0.0f, 0.0f}, Vector3{1.0f, 0.0f, 0.0f},
                     Vector3{0.0f, -1.0f, 0.0f}, Vector3{0.0f, 1.0f, 0.0f},
                     Vector3{0.0f, 0.0f, -1.0f}, Vector3{0.0f, 0.0f, 1.0f}}};

                bool hasNeighbourAir = false;
                for (const Vector3& offset : neighbourCubesOffsets) {
                    const int neighbourX = x + static_cast<int>(offset.x);
                    const int neighbourY = y + static_cast<int>(offset.y);
                    const int neighbourZ = z + static_cast<int>(offset.z);

                    // If the neighbouring block is air, we need to render this block
                    if (neighbourX < 0 || neighbourX >= MAP_WIDTH || neighbourY < 0 ||
                        neighbourY >= MAP_HEIGHT || neighbourZ < 0 || neighbourZ >= MAP_DEPTH ||
                        world_[neighbourX][neighbourY][neighbourZ] == BlockType::BLOCK_AIR) {
                        hasNeighbourAir = true;
                        break;
                    }
                }

                if (!hasNeighbourAir) {
                    continue;  // Skip rendering this block if no air neighbour
                }

                const Vector3 pos = {static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f,
                                     static_cast<float>(z) + 0.5f};

                Matrix model = MatrixTranslate(pos.x, pos.y, pos.z);

                if (world_[x][y][z] == BlockType::BLOCK_GRASS) {
                    grassTransforms.push_back(model);
                } else if (world_[x][y][z] == BlockType::BLOCK_DIRT) {
                    dirtTransforms.push_back(model);
                } else if (world_[x][y][z] == BlockType::BLOCK_STONE) {
                    stoneTransforms.push_back(model);
                } else {
                    throw std::runtime_error(
                        std::format("Unknown block type at ({}, {}, {}): got {}", x, y, z,
                                    static_cast<int>(world_[x][y][z])));
                }
            }
        }
    }

    const float endTime = GetTime();
    std::printf(
        "Generated %d blocks in %.4f seconds\n",
        static_cast<int>(grassTransforms.size() + dirtTransforms.size() + stoneTransforms.size()),
        endTime - startTime);
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