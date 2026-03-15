#pragma once
#include <cstdint>

enum class LocomotionState: uint8_t {
    Idle,
    Forward,
    Backward,
    StrafeLeft,
    StrafeRight,
    Jump,
    Fall,
    Land,
    Count
};

struct RecvLocomotionState {
    LocomotionState state{};
};

// The name of the animation baked into the model
constexpr const char* anim_names[(std::size_t)LocomotionState::Count] = {
    "idle",
    "forward",
    "forward",
    "forward",
    "forward",
    "idle",
    "idle",
    "idle"
};

// Just a printable label for the animations
constexpr const char* anim_labels[(std::size_t)LocomotionState::Count] = {
    "Idle",
    "Forward",
    "Backward",
    "Strafe Left",
    "Strafe Right",
    "Jump",
    "Fall",
    "Land"
};


struct AnimationTimer {
    float time = 0;
};
