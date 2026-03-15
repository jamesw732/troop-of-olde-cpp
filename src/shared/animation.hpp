#pragma once
#include "components.hpp"
#include "animation_components.hpp"

inline LocomotionState get_locomotion_state(const MovementInput& input) {
    if (input.get_backward()) {
        return LocomotionState::Backward;
    }
    if (input.get_forward()) {
        return LocomotionState::Forward;
    }
    if (input.get_strafe_r()) {
        return LocomotionState::StrafeRight;
    }
    if (input.get_strafe_l()) {
        return LocomotionState::StrafeLeft;
    }
    return LocomotionState::Idle;
}
