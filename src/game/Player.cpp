#include "Player.hpp"

#include "common/UtilityStructures.hpp"
#include "raylib.h"
#include "raymath.h"

void Player::update() {
    updatePosition();
    updateCamera();
}

void Player::updatePosition() {
    const float deltaTime = GetFrameTime();

    if (IsKeyPressed(KEY_LEFT_SHIFT)) {
        if (movementMode_ == MovementMode::WALKING) {
            movementMode_ = MovementMode::RUNNING;
        } else if (movementMode_ == MovementMode::RUNNING) {
            movementMode_ = MovementMode::SUPER_RUNNING;
        } else {
            movementMode_ = MovementMode::WALKING;
        }
    }

    const float movementSpeed = (movementMode_ == MovementMode::WALKING)   ? walkingSpeed_
                                : (movementMode_ == MovementMode::RUNNING) ? runningSpeed_
                                                                           : superRunningSpeed_;
    const float movementDistance = deltaTime * movementSpeed;

    Vector2 movement2DRelative = {static_cast<float>(IsKeyDown(KEY_D) - IsKeyDown(KEY_A)),
                                  static_cast<float>(IsKeyDown(KEY_W) - IsKeyDown(KEY_S))};
    movement2DRelative = Vector2Scale(Vector2Normalize(movement2DRelative), movementDistance);
    const Vector2 movement2D = Vector2Rotate(movement2DRelative, cameraYaw_);
    position_.x += movement2D.x;
    position_.y += movement2D.y;

    float verticalMovementDirection = 0.0f;
    if (IsKeyDown(KEY_SPACE))
        verticalMovementDirection = 1.0f;
    else if (IsKeyDown(KEY_LEFT_CONTROL))
        verticalMovementDirection = -1.0f;
    position_.z += verticalMovementDirection * movementDistance * verticalSpeedMultiplier_;
}

void Player::updateCamera() {
    camera_.position = position_;

    const Vector2 mouseDelta = GetMouseDelta();
    cameraYaw_ -= mouseDelta.x * cameraSensitivity_;
    cameraPitch_ -= mouseDelta.y * cameraSensitivity_;
    cameraPitch_ = Clamp(cameraPitch_, -89.0f / 180.0f * M_PI,
                         89.0f / 180.0f * M_PI);  // Limit pitch to avoid flipping

    const Vector3 direction = {
        -sinf(cameraYaw_) * cosf(cameraPitch_),
        cosf(cameraYaw_) * cosf(cameraPitch_),
        sinf(cameraPitch_),
    };
    camera_.target = Vector3Add(camera_.position, direction);

    camera_.fovy = IsKeyDown(KEY_C) ? cameraFovZoom_ : cameraFov_;
}