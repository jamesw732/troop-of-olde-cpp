#pragma once
#include "flecs.h"

#include "../shared/animation.hpp"
#include "../shared/const.hpp"
#include "input.hpp"

inline void register_animation_tick_system(flecs::world& world, InputBuffer& input_buffer) {
    world.system<LocalPlayer, LocomotionState, AnimationTimer>()
        .interval(MOVE_UPDATE_RATE)
        .each([&input_buffer] (LocalPlayer, LocomotionState& movement_state, AnimationTimer& timer) {
            std::optional<MovementInput> opt = input_buffer.back();
            if (!opt) {
                return;
            }
            MovementInput input = *opt;
            LocomotionState new_movement_state = get_locomotion_state(input);
            /* std::cout << anim_labels[(size_t) new_movement_state] << "\n"; */
            if (movement_state == new_movement_state) {
                return;
            }
            movement_state = new_movement_state;
            timer.time = 0;
        }
    );
}

inline void register_animation_recv_system(flecs::world& world) {
    world.system<RecvLocomotionState, LocomotionState, AnimationTimer>()
        .without<LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([] (RecvLocomotionState recv_state, LocomotionState& movement_state, AnimationTimer& timer) {
            /* std::cout << anim_labels[(size_t) recv_state.state] << "\n"; */
            if (recv_state.state == movement_state) {
                return;
            }
            movement_state = recv_state.state;
            timer.time = 0;
        }
    );
}

inline void register_character_pose_system(flecs::world& world) {
    world.system<ModelPointer, ModelAnimations, LocomotionState, AnimationTimer>()
        .each([] (ModelPointer& model, ModelAnimations& anims,
                  const LocomotionState& movement_state, AnimationTimer& timer)
        {
            std::string anim_name = anim_names[(size_t) movement_state];
            ModelAnimation anim = anims.map->at(anim_name);
            UpdateModelAnimation(*model.model, anim, timer.time * ANIMATION_FPS);
            timer.time += GetFrameTime();
        }
    );
}
