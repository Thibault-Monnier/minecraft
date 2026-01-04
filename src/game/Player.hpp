#pragma once
#include "raylib.h"

struct Keybinds {
    int forward;
    int backward;
    int left;
    int right;
    int jump;
    int crouch;
    int run;
    int superRun;
    int zoomIn;
};

class Player {
   public:
    Player() {
        camera_ = {
            .position = {0.0f, 0.0f, 0.0f},
            .target = {0.0f, 1.0f, 0.0f},
            .up = {0.0f, 0.0f, 1.0f},
            .fovy = cameraFov_,
            .projection = CAMERA_PERSPECTIVE,
        };
    }

    void update();

    void setPosition(const Vector3& position) { position_ = position; }

    [[nodiscard]] const Vector3& getPosition() const { return position_; }
    [[nodiscard]] const Camera& getCamera() const { return camera_; }

   private:
    constexpr static float walkingSpeed_ = 4.3f;
    constexpr static float runningSpeed_ = 17.5f;
    constexpr static float superRunningSpeed_ = 50.0f;
    constexpr static float verticalSpeedMultiplier_ = 1.5f;

    enum class MovementMode {
        WALKING,
        RUNNING,
        SUPER_RUNNING,
    };
    MovementMode movementMode_ = MovementMode::WALKING;

    constexpr static float cameraSensitivity_ = 0.0025f;
    constexpr static float cameraFov_ = 80.0f;
    constexpr static float cameraFovZoom_ = 20.0f;

    float cameraYaw_ = 0.0f;
    float cameraPitch_ = 0.0f;

    constexpr static Keybinds keybinds_{
        .forward = KEY_W,
        .backward = KEY_S,
        .left = KEY_A,
        .right = KEY_D,
        .jump = KEY_SPACE,
        .crouch = KEY_LEFT_CONTROL,
        .run = KEY_LEFT_SHIFT,
        .superRun = KEY_LEFT_ALT,
        .zoomIn = KEY_C,
    };

    Camera camera_{};
    Vector3 position_{};

    void updatePosition();
    void updateCamera();
};
