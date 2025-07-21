#include "Player.hpp"

#include "raylib.h"
#include "raymath.h"

void Player::update() {
    updatePosition();
    updateCamera();
}

void Player::updatePosition() {
    const float deltaTime = GetFrameTime();

    if (IsKeyPressed(KEY_LEFT_SHIFT)) isRunning_ = !isRunning_;
    const float movementDistance = deltaTime * (isRunning_ ? runningSpeed_ : walkingSpeed_);

    Vector2 movement2DRelative = {0.0f, 0.0f};
    if (IsKeyDown(KEY_W))
        movement2DRelative.x = 1.0f;
    else if (IsKeyDown(KEY_S))
        movement2DRelative.x = -1.0f;
    if (IsKeyDown(KEY_A))
        movement2DRelative.y = 1.0f;
    else if (IsKeyDown(KEY_D))
        movement2DRelative.y = -1.0f;

    movement2DRelative = Vector2Normalize(movement2DRelative);

    const Vector2 forward2D = {cosf(cameraYaw_), sinf(cameraYaw_)};
    const Vector2 right2D = {forward2D.y, -forward2D.x};

    position_.x +=
        (forward2D.x * movement2DRelative.x + right2D.x * movement2DRelative.y) * movementDistance;
    position_.z +=
        (forward2D.y * movement2DRelative.x + right2D.y * movement2DRelative.y) * movementDistance;

    float verticalMovementDirection = 0.0f;
    if (IsKeyDown(KEY_SPACE))
        verticalMovementDirection = 1.0f;
    else if (IsKeyDown(KEY_LEFT_CONTROL))
        verticalMovementDirection = -1.0f;

    position_.y += verticalMovementDirection * movementDistance * verticalSpeedMultiplier_;
}

void Player::updateCamera() {
    camera_.position = position_;

    const Vector2 mouseDelta = GetMouseDelta();
    cameraYaw_ += mouseDelta.x * cameraSensitivity_;
    cameraPitch_ -= mouseDelta.y * cameraSensitivity_;
    cameraPitch_ = Clamp(cameraPitch_, -89.0f / 180.0f * M_PI,
                         89.0f / 180.0f * M_PI);  // Limit pitch to avoid flipping

    const Vector3 direction = {cosf(cameraYaw_) * cosf(cameraPitch_), sinf(cameraPitch_),
                               sinf(cameraYaw_) * cosf(cameraPitch_)};

    camera_.target = Vector3Add(camera_.position, direction);

    camera_.fovy = IsKeyDown(KEY_C) ? cameraFovZoom_ : cameraFov_;
}