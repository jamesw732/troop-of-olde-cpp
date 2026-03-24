#pragma once
#include "flecs.h"

#include "../shared/animation.hpp"
#include "../shared/const.hpp"
#include "input.hpp"

// Increments animation state machine for the local player, once per movement tick
inline void register_animation_tick_system(flecs::world& world, InputBuffer& input_buffer) {
    // TODO: Really seems unnecessary to send whole input buffer
    // Maybe go back to storing the MovementInput for the current tick?
    world.system<CurLocomotionState, AnimationFrame,
                 CurLocomotionPose, PrevLocomotionPose, LocomotionBlendFactor>()
        .with<LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([&input_buffer] (CurLocomotionState& movement_state, AnimationFrame& frame,
                    CurLocomotionPose cur_pose, PrevLocomotionPose& prev_pose,
                    LocomotionBlendFactor& alpha) {
            if (input_buffer.empty()) return;
            MovementInput input = input_buffer.back();
            LocomotionState new_movement_state = get_locomotion_state(input);
            if (movement_state.state == new_movement_state) {
                return;
            }
            // If new state, save current pose to previous pose
            prev_pose.pose = cur_pose.pose;
            movement_state.state = new_movement_state;
            frame.frame = 0;
            alpha.val = 0;
        }
    );
}

// Updates animation state for remote players
inline void register_animation_recv_system(flecs::world& world) {
    world.system<RecvLocomotionState, CurLocomotionState, AnimationFrame,
                 CurLocomotionPose, PrevLocomotionPose, LocomotionBlendFactor>()
        .without<LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([] (RecvLocomotionState recv_state,
                  CurLocomotionState& movement_state, AnimationFrame& frame,
                  CurLocomotionPose cur_pose, PrevLocomotionPose& prev_pose,
                  LocomotionBlendFactor& alpha) {
            /* std::cout << anim_labels[(size_t) recv_state.state] << "\n"; */
            if (recv_state.state == movement_state.state) {
                return;
            }
            // If new state, move current state and reset
            prev_pose.pose = cur_pose.pose;
            movement_state.state = recv_state.state;
            frame.frame = 0;
            alpha.val = 0;
        }
    );
}

// Increments animation frame
inline void register_animation_frame_system(flecs::world& world) {
    world.system<AnimationFrame>()
        .each([] (AnimationFrame& frame) {
            frame.frame += GetFrameTime() * ANIMATION_FPS;
        }
    );
}

// Sets the locomotion pose given the locomotion state and frame
inline void register_locomotion_pose_system(flecs::world& world) {
    world.system<CurLocomotionPose, CurLocomotionState, AnimationFrame, ModelAnimations>()
        .each([] (CurLocomotionPose& pose, CurLocomotionState state,
                  AnimationFrame frame, ModelAnimations anims) {
            std::string anim_name = anim_names[(size_t) state.state];
            ModelAnimation anim = anims.map->at(anim_name);
            pose.pose = sample_pose_from_anim(anim, frame.frame);
        }
    );
}

// Sets the initial render pose so other systems can blend onto it
inline void register_set_render_pose_system(flecs::world& world) {
    world.system<RenderPose, CurLocomotionPose>()
        .each([] (RenderPose& render_pose, CurLocomotionPose cur_pose) {
            render_pose.pose = cur_pose.pose;
        }
    );
}

// Blends render pose with previous locomotion pose to smooth out animation
inline void register_locomotion_blend_system(flecs::world& world) {
    world.system<RenderPose, PrevLocomotionPose, LocomotionBlendFactor>()
        .each([] (RenderPose& render_pose, PrevLocomotionPose prev_pose,
                  LocomotionBlendFactor& alpha) {
            render_pose.pose = blend_poses(prev_pose.pose, render_pose.pose, alpha.val);
            alpha.val = fmin(1.0, alpha.val + GetFrameTime() / ANIMATION_BLEND_TIME);
        }
    );
}
