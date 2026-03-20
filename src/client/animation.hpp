#pragma once
#include "flecs.h"

#include "../shared/animation.hpp"
#include "../shared/const.hpp"
#include "input.hpp"

inline void register_animation_tick_system(flecs::world& world, InputBuffer& input_buffer) {
    world.system<CurLocomotionState, AnimationFrame,
                 PrevLocomotionState, PrevAnimationFrame, BlendFactor>()
        .with<LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([&input_buffer] (CurLocomotionState& movement_state, AnimationFrame& frame,
                    PrevLocomotionState& prev_movement_state, PrevAnimationFrame& prev_frame,
                    BlendFactor& alpha) {
            if (input_buffer.empty()) return;
            MovementInput input = input_buffer.back();
            LocomotionState new_movement_state = get_locomotion_state(input);
            /* std::cout << anim_labels[(size_t) new_movement_state] << "\n"; */
            if (movement_state.state == new_movement_state) {
                return;
            }
            // If new state, move current state and reset
            prev_movement_state.state = movement_state.state;
            prev_frame.frame = frame.frame;
            movement_state.state = new_movement_state;
            frame.frame = 0;
            alpha.val = 0;
        }
    );
}

inline void register_animation_recv_system(flecs::world& world) {
    world.system<RecvLocomotionState, CurLocomotionState, AnimationFrame,
                 PrevLocomotionState, PrevAnimationFrame, BlendFactor>()
        .without<LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([] (RecvLocomotionState recv_state,
                  CurLocomotionState& movement_state, AnimationFrame& frame,
                  PrevLocomotionState& prev_movement_state, PrevAnimationFrame& prev_frame,
                  BlendFactor& alpha) {
            /* std::cout << anim_labels[(size_t) recv_state.state] << "\n"; */
            if (recv_state.state == movement_state.state) {
                return;
            }
            // If new state, move current state and reset
            prev_movement_state.state = movement_state.state;
            prev_frame.frame = frame.frame;
            movement_state.state = recv_state.state;
            frame.frame = 0;
            alpha.val = 0;
        }
    );
}

inline flecs::system register_animation_frame_system(flecs::world& world, flecs::entity phase) {
    return world.system<AnimationFrame>()
        .kind(phase)
        .each([] (AnimationFrame& frame) {
            frame.frame += GetFrameTime() * ANIMATION_FPS;
        }
    );
}

inline flecs::system register_animation_blend_system(flecs::world& world, flecs::entity phase) {
    return world.system<BlendFactor>()
        .kind(phase)
        .each([] (BlendFactor& alpha) {
            alpha.val = fmin(1.0, alpha.val + GetFrameTime() / ANIMATION_BLEND_TIME);
        }
    );
}
