#pragma once
#include "flecs.h"

#include "../shared/animation.hpp"
#include "../shared/const.hpp"
#include "input.hpp"

// Increments animation state machine for the local player, once per movement tick
inline void register_animation_tick_system(flecs::world& world, InputBuffer& input_buffer) {
    // TODO: Really seems unnecessary to send whole input buffer
    // Maybe go back to storing the MovementInput for the current tick?
    world.system<LocomotionBlendSpace, AnimationFrame,
                 CurLocomotionPose, PrevLocomotionPose, LocomotionBlendFactor>()
        .with<LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([&input_buffer] (LocomotionBlendSpace& blend_space, AnimationFrame& frame,
                    CurLocomotionPose cur_pose, PrevLocomotionPose& prev_pose,
                    LocomotionBlendFactor& alpha) {
            if (input_buffer.empty()) return;
            MovementInput input = input_buffer.back();
            LocomotionBlendSpace new_blend_space = get_blend_space_from_input(input);
            if (new_blend_space.is_close(blend_space)) {
                return;
            }
            // If new state, save current pose to previous pose
            prev_pose.pose = cur_pose.pose;
            blend_space = new_blend_space;
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
            // TODO: Update this to use blend space
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

inline Pose sample_pose_from_state(LocomotionState state, float frame, ModelAnimations anims) {
    std::string anim_name = anim_names[(size_t) state];
    ModelAnimation anim = anims.map->at(anim_name);
    return sample_pose_from_anim(anim, frame);
}

// Sets the locomotion pose given the locomotion state and frame
inline void register_locomotion_pose_system(flecs::world& world) {
    world.system<LocomotionBlendSpace, AnimationFrame, ModelAnimations, CurLocomotionPose>()
        .each([] (LocomotionBlendSpace blend_space, AnimationFrame frame,
                  ModelAnimations anims, CurLocomotionPose& cur_pose) {
            Pose idle_pose = sample_pose_from_state(LocomotionState::Idle, frame.frame, anims);
            if (is_close(blend_space.total(), 0)) {
                cur_pose.pose = idle_pose;
                return;
            }
            if (blend_space.wF > 0.0f) {
                cur_pose.pose = sample_pose_from_state(LocomotionState::Forward, frame.frame, anims);
                return;
            }
            if (blend_space.wB > 0.0f) {
                cur_pose.pose = sample_pose_from_state(LocomotionState::Backward, frame.frame, anims);
                return;
            }
            if (blend_space.wL > 0.0f) {
                cur_pose.pose = sample_pose_from_state(LocomotionState::StrafeLeft, frame.frame, anims);
                return;
            }
            if (blend_space.wR > 0.0f) {
                cur_pose.pose = sample_pose_from_state(LocomotionState::StrafeRight, frame.frame, anims);
                return;
            }
            // TODO: Improve synchronization between animations
            // The below blending code seems to work, but the animations do not render correctly
            // due to synchronization issues
            // This will also include synchronizing the number of keyframes in the animation
            /* Pose fwd_pose = sample_pose_from_state(LocomotionState::Forward, frame.frame, anims); */
            /* Pose bwd_pose = sample_pose_from_state(LocomotionState::Backward, frame.frame, anims); */
            /* Pose left_pose = sample_pose_from_state(LocomotionState::StrafeLeft, frame.frame, anims); */
            /* Pose right_pose = sample_pose_from_state(LocomotionState::StrafeRight, frame.frame, anims); */
            /* Pose out; */
            /* for (int i = 0; i < fwd_pose.transforms.size(); i++) { */
            /*     Transform transform; */
            /*     transform.translation = */ 
            /*           fwd_pose.transforms[i].translation * blend_space.wF */
            /*         + bwd_pose.transforms[i].translation * blend_space.wB */
            /*         + left_pose.transforms[i].translation * blend_space.wL */ 
            /*         + right_pose.transforms[i].translation * blend_space.wR ; */
            /*     transform.rotation = */ 
            /*           fwd_pose.transforms[i].rotation * blend_space.wF */
            /*         + bwd_pose.transforms[i].rotation * blend_space.wB */
            /*         + left_pose.transforms[i].rotation * blend_space.wL */
            /*         + right_pose.transforms[i].rotation * blend_space.wR; */
            /*     transform.rotation = QuaternionNormalize(transform.rotation); */
            /*     transform.scale = */ 
            /*         fwd_pose.transforms[i].scale * blend_space.wF */
            /*       + bwd_pose.transforms[i].scale * blend_space.wB */
            /*       + left_pose.transforms[i].scale * blend_space.wL */
            /*       + right_pose.transforms[i].scale * blend_space.wR; */

            /*     out.transforms.push_back(transform); */
            /* } */
            /* cur_pose.pose = out; */
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
