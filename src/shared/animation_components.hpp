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

// Temporary names compatible with bad model
constexpr const char* anim_names[(std::size_t)LocomotionState::Count] = {
    "Idle",
    "RunCycle",
    "RunCycle",
    "RunCycle",
    "RunCycle",
    "Idle",
    "Idle",
    "Idle"
};
// The name of the animation baked into the model
/* constexpr const char* anim_names[(std::size_t)LocomotionState::Count] = { */
/*     "idle", */
/*     "forward", */
/*     "forward", */
/*     "forward", */
/*     "forward", */
/*     "idle", */
/*     "idle", */
/*     "idle" */
/* }; */

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
