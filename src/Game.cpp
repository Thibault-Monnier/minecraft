#include "Game.hpp"

#include <format>
#include <iostream>
#include <ranges>

#define RLIGHTS_IMPLEMENTATION
#include "Perf.hpp"
#include "UtilityStructures.hpp"
#include "raylib.h"
#include "raymath.h"
#include "rlights.h"

bool Game::isPositionInRenderDistance(const Vector3& position) const {
    constexpr float maxDistanceSq =
        RENDER_DISTANCE * RENDER_DISTANCE * Chunk::CHUNK_SIZE * Chunk::CHUNK_SIZE;
    return Vector3DistanceSqr(position, player_.getPosition()) < maxDistanceSq;
}

void Game::drawSky() {
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();

    DrawRectangleGradientV(0, 0, screenWidth, screenHeight, {135, 206, 235, 255},
                           {65, 105, 225, 255});
}

void Game::drawCursor() {
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();
    const Vector2 center = {static_cast<float>(screenWidth) / 2.0f,
                            static_cast<float>(screenHeight) / 2.0f};

    DrawCircleLinesV(center, 7.0f, BLACK);  // Draw a circle around the cursor
    DrawCircleLinesV(center, 1.0f, BLACK);  // Draw a dot in the center
}

void Game::drawFps() {
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();
    DrawText(TextFormat("FPS: %i", GetFPS()), screenWidth - 100, screenHeight - 30, 20, BLACK);
}

void Game::drawPositionInfo(const Vector3& position) {
    DrawRectangle(10, 10, 200, 80, Fade(BLACK, 0.35f));  // Semi-transparent background
    DrawRectangleLines(10, 10, 200, 80, BLACK);          // Border around the rectangle
    DrawText(TextFormat("X: %.2f\nY: %.2f\nZ: %.2f", position.x, position.y, position.z), 20, 20,
             20, BLACK);
}

void Game::draw() const {
    const Camera& camera_ = player_.getCamera();

    SetShaderValue(terrainShader_, terrainShader_.locs[SHADER_LOC_VECTOR_VIEW], &camera_.position,
                   SHADER_UNIFORM_VEC3);

    BeginDrawing();
    ClearBackground(RAYWHITE);

    drawSky();

    BeginMode3D(camera_);

    // const auto startTime = static_cast<float>(GetTime());
    for (const auto& chunk : world_ | std::views::values) {
        if (isPositionInRenderDistance(chunk->getCenterPosition())) chunk->render();
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

std::array<OptionalRef<Chunk>, 6> Game::findAdjacentChunks(const Chunk& chunk) const {
    std::array<OptionalRef<Chunk>, 6> adjacentChunks{};
    const int x = chunk.getX();
    const int y = chunk.getY();
    const int z = chunk.getZ();

    auto addAdjacentChunk = [&](const int dx, const int dy, const int dz, const size_t index) {
        const auto it = world_.find({x + dx, y + dy, z + dz});
        adjacentChunks[index] =
            (it != world_.end()) ? OptionalRef<Chunk>{*it->second} : std::nullopt;
    };
    addAdjacentChunk(+1, 0, 0, 0);  // Positive X
    addAdjacentChunk(-1, 0, 0, 1);  // Negative X
    addAdjacentChunk(0, +1, 0, 2);  // Positive Y
    addAdjacentChunk(0, -1, 0, 3);  // Negative Y
    addAdjacentChunk(0, 0, +1, 4);  // Positive Z
    addAdjacentChunk(0, 0, -1, 5);  // Negative Z

    return adjacentChunks;
}

Chunk& Game::generateChunk(const Vector3Int& pos) {
    const Chunk chunk{
        pos.x, pos.y, pos.z, cubeMesh_, materialGrass_, materialDirt_, materialStone_,
    };
    auto [it, _] = world_.emplace(pos, std::make_unique<Chunk>(chunk));
    it->second->generate(SEED, MAP_HEIGHT_BLOCKS);
    return *it->second;
};

void Game::generateChunkTransforms(Chunk& chunk) const {
    const auto adjacentChunks = findAdjacentChunks(chunk);
    chunk.generateTransforms(adjacentChunks[0],   // Positive X
                             adjacentChunks[1],   // Negative X
                             adjacentChunks[2],   // Positive Y
                             adjacentChunks[3],   // Negative Y
                             adjacentChunks[4],   // Positive Z
                             adjacentChunks[5]);  // Negative Z
}

void Game::updateTerrain() {
    const double startTime = GetTime();

    std::unordered_set<const Chunk*> chunksToUpdateTransforms;

    const auto [playerX, _, playerZ] = player_.getPosition();
    const auto playerChunkX = static_cast<int>(playerX / Chunk::CHUNK_SIZE);
    const auto playerChunkZ = static_cast<int>(playerZ / Chunk::CHUNK_SIZE);
    for (int x = -RENDER_DISTANCE; x < RENDER_DISTANCE; x++) {
        for (int chunkY = 0; chunkY < MAP_HEIGHT_BLOCKS / Chunk::CHUNK_SIZE; chunkY++) {
            for (int z = -RENDER_DISTANCE; z < RENDER_DISTANCE; z++) {
                const int chunkX = playerChunkX + x;
                const int chunkZ = playerChunkZ + z;

                if (!isPositionInRenderDistance(
                        Vector3{static_cast<float>(chunkX * Chunk::CHUNK_SIZE) + 0.5f,
                                static_cast<float>(chunkY * Chunk::CHUNK_SIZE) + 0.5f,
                                static_cast<float>(chunkZ * Chunk::CHUNK_SIZE) +
                                    0.5f})) {  // Center of the chunk
                    continue;
                }

                const Vector3Int position = {chunkX, chunkY, chunkZ};
                if (world_.contains(position)) {
                    continue;
                }

                Chunk& chunk = generateChunk(position);

                auto adjacentChunks = findAdjacentChunks(chunk);
                chunksToUpdateTransforms.insert(&chunk);
                for (const auto& adjacentChunk : adjacentChunks) {
                    if (adjacentChunk &&
                        isPositionInRenderDistance(adjacentChunk->get().getCenterPosition())) {
                        chunksToUpdateTransforms.insert(&adjacentChunk->get());
                    }
                }
            }
        }
    }

    std::cout << "Chunks to update transforms: " << chunksToUpdateTransforms.size() << std::endl;

    const double intermediateTime = GetTime();
    for (const auto* chunk : chunksToUpdateTransforms) {
        generateChunkTransforms(*const_cast<Chunk*>(chunk));
    }

    const double endTime = GetTime();
    std::cout << "Terrain update time: " << (endTime - startTime) * 1000.0f << " ms" << std::endl;
    std::cout << "Chunk transforms generation time: " << (endTime - intermediateTime) * 1000.0f
              << " ms" << std::endl;
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

    // Generate spawn chunks first to know the starting position for accurate render distance
    const double startTime = GetTime();
    for (int y = 0; y < MAP_HEIGHT_BLOCKS / Chunk::CHUNK_SIZE; y++) {
        generateChunk({0, y, 0});
    }

    // Determine the starting position
    int startY = 0;
    while (world_.at({0, startY / Chunk::CHUNK_SIZE, 0})
               ->getData()[0][startY % Chunk::CHUNK_SIZE][0] != BlockType::BLOCK_AIR) {
        startY++;
    }
    startY += 2;                                                    // Start above the ground
    player_.setPosition({0.5f, static_cast<float>(startY), 0.5f});  // Middle of the block

    // Generate the remaining chunks
    updateTerrain();
    const double endTime = GetTime();
    std::cout << "Terrain generation time: " << (endTime - startTime) * 1000.0f << " ms"
              << std::endl;

    const double transformStartTime = GetTime();
    std::cout << "World size: " << world_.size() << std::endl;
    for (auto& chunk : world_ | std::views::values) {
        generateChunkTransforms(*chunk);
    }
    const double transformsEndTime = GetTime();
    std::cout << "Block transforms generation time: "
              << (transformsEndTime - transformStartTime) * 1000.0f << " ms" << std::endl;

    std::cout << "Generated " << world_.size() << " chunks in "
              << (transformsEndTime - startTime) * 1000.0f << " ms" << std::endl;
}

void Game::run() {
    while (!WindowShouldClose()) {
        player_.update();

        updateTerrain();

        draw();
    }
}