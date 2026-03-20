#pragma once
#include <cstdint>
#include <iostream>
#include <optional>

#include "flecs.h"

#include "../shared/animation.hpp"
#include "../shared/components.hpp"
#include "../shared/const.hpp"
#include "components.hpp"

inline void register_animation_tick_system(flecs::world& world) {
    world.system<InputBuffer, LocomotionState>()
        .interval(MOVE_UPDATE_RATE)
        .each([] (InputBuffer& input_buffer, LocomotionState& movement_state) {
            if (input_buffer.empty()) return;
            MovementInput input = input_buffer.back();
            LocomotionState new_movement_state = get_locomotion_state(input);
            movement_state = new_movement_state;
        }
    );
}
