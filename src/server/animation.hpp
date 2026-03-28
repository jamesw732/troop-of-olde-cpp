#pragma once
#include "flecs.h"

#include "../shared/animation.hpp"
#include "../shared/components.hpp"
#include "../shared/const.hpp"
#include "components.hpp"

inline void register_animation_tick_system(flecs::world& world) {
    world.system<InputBuffer, LocomotionBlendSpace>()
        .interval(MOVE_UPDATE_RATE)
        .each([] (InputBuffer& input_buffer, LocomotionBlendSpace& blend_space) {
            if (input_buffer.empty()) return;
            MovementInput input = input_buffer.back();
            blend_space = get_blend_space_from_input(input);
        }
    );
}
