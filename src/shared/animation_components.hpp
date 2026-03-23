#pragma once
#include <cstdint>

#include "raylib.h"

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

// Temporary names compatible with bad model
// TODO: Blend between movement directions
constexpr const char* anim_names[(std::size_t)LocomotionState::Count] = {
    "Idle",
    "RunForward",
    "RunBackward",
    "StrafeLeft",
    "StrafeRight",
    "Jump",
    "Idle",
    "Idle"
};

// Just a printable label for the animations
constexpr const char* anim_labels[(std::size_t)LocomotionState::Count] = {
    "Idle",
    "Run Forward",
    "Run Backward",
    "Strafe Left",
    "Strafe Right",
    "Jump",
    "Fall",
    "Land"
};

struct Pose{
    int bone_count = 0;
    ModelAnimPose transforms = nullptr;
};

struct CurLocomotionState {
    LocomotionState state{};
};

struct PrevLocomotionState {
    LocomotionState state{};
};

struct RecvLocomotionState {
    LocomotionState state{};
};

struct AnimationFrame {
    float frame = 0;
};

struct PrevAnimationFrame {
    float frame = 0;
};

struct BlendFactor {
    float val = 0;
};
