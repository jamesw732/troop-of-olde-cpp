#pragma once
#include "../shared/animation.hpp"
#include "../shared/const.hpp"
#include "input.hpp"

// Increments animation state machine for the local player, once per movement tick
inline void register_locomotion_tick_system(flecs::world& world, InputBuffer& input_buffer) {
    // TODO: Really seems unnecessary to send whole input buffer
    // Maybe go back to storing the MovementInput for the current tick?
    world.system<LocomotionBlendSpace, LocomotionPhase,
                 CurLocomotionPose, PrevLocomotionPose, LocomotionBlendFactor>()
        .with<LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([&input_buffer] (LocomotionBlendSpace& blend_space, LocomotionPhase& phase,
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
            alpha.val = 0;
            if (new_blend_space.has_common_weight(blend_space)) {
                return;
            }
            phase.phase = 0;
        }
    );
}

// Updates animation state for remote players
inline void register_animation_recv_system(flecs::world& world) {
    world.system<RecvLocomotionBlendSpace, LocomotionBlendSpace, LocomotionPhase,
                 CurLocomotionPose, PrevLocomotionPose, LocomotionBlendFactor>()
        .without<LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([] (RecvLocomotionBlendSpace recv_blend_space,
                  LocomotionBlendSpace& blend_space, LocomotionPhase& phase,
                  CurLocomotionPose cur_pose, PrevLocomotionPose& prev_pose,
                  LocomotionBlendFactor& alpha) {
            LocomotionBlendSpace new_blend_space = recv_blend_space.blend_space;
            if (new_blend_space.is_close(blend_space)) {
                return;
            }
            // If new state, move current state and reset
            prev_pose.pose = cur_pose.pose;
            blend_space = recv_blend_space.blend_space;
            phase.phase = 0;
            if (new_blend_space.has_common_weight(blend_space)) {
                return;
            }
            alpha.val = 0;
        }
    );
}

// Increments animation frame
inline void register_locomotion_phase_system(flecs::world& world) {
    world.system<LocomotionPhase>()
        .each([] (LocomotionPhase& phase) {
            phase.phase += GetFrameTime() * 5 / 3; // This should eventually scale with character speed
            phase.phase = fmodf(phase.phase, 1.0f);
        }
    );
}

// Sets the locomotion pose given the locomotion state and frame
inline void register_locomotion_pose_system(flecs::world& world) {
    world.system<LocomotionBlendSpace, LocomotionPhase, ModelAnimations, CurLocomotionPose>()
        .each([] (LocomotionBlendSpace blend_space, LocomotionPhase phase,
                  ModelAnimations anims, CurLocomotionPose& cur_pose) {
            Pose idle_pose = sample_cyclic_pose_from_state(LocomotionState::Idle, phase.phase, anims);
            if (is_close(blend_space.total(), 0)) {
                cur_pose.pose = idle_pose;
                return;
            }
            Pose fwd_pose = sample_cyclic_pose_from_state(LocomotionState::Forward, phase.phase, anims);
            Pose bwd_pose = sample_cyclic_pose_from_state(LocomotionState::Backward, phase.phase, anims);
            Pose left_pose = sample_cyclic_pose_from_state(LocomotionState::StrafeLeft, phase.phase, anims);
            Pose right_pose = sample_cyclic_pose_from_state(LocomotionState::StrafeRight, phase.phase, anims);
            Pose out;
            for (int i = 0; i < fwd_pose.transforms.size(); i++) {
                Transform transform;
                transform.translation =
                      fwd_pose.transforms[i].translation * blend_space.wF
                    + bwd_pose.transforms[i].translation * blend_space.wB
                    + left_pose.transforms[i].translation * blend_space.wL
                    + right_pose.transforms[i].translation * blend_space.wR;
                Quaternion base = fwd_pose.transforms[i].rotation;
                Quaternion qF = base;
                Quaternion qB = align(base, bwd_pose.transforms[i].rotation);
                Quaternion qL = align(base, left_pose.transforms[i].rotation);
                Quaternion qR = align(base, right_pose.transforms[i].rotation);
                transform.rotation =
                      qF * blend_space.wF
                    + qB * blend_space.wB
                    + qL * blend_space.wL
                    + qR * blend_space.wR;
                transform.rotation = QuaternionNormalize(transform.rotation);
                transform.scale =
                    fwd_pose.transforms[i].scale * blend_space.wF
                  + bwd_pose.transforms[i].scale * blend_space.wB
                  + left_pose.transforms[i].scale * blend_space.wL
                  + right_pose.transforms[i].scale * blend_space.wR;

                out.transforms.push_back(transform);
            }
            cur_pose.pose = out;
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
