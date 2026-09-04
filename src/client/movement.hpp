#pragma once
#include <unordered_map>

#include "camera.hpp"
#include "flecs.h"
#include "raylib.h"
#include "raymath.h"

#include "input.hpp"
#include "components.hpp"
#include "network.hpp"
#include "../shared/movement.hpp"
#include "../shared/components.hpp"
#include "../shared/const.hpp"
#include "../shared/physics.hpp"
#include "../shared/raylib-util.hpp"
#include "../shared/serialize.hpp"
#include "../shared/util.hpp"

/* #define DISABLE_SERVER */

// MOVEMENT INPUT SYSTEMS
inline void register_movement_input_aggregate_system(flecs::world& world) {
    // Build up the active MovementInput per-frame, to be processed per-tick
    world.system<MovementInput>()
        .each([] (MovementInput& input) {
            int16_t mouse_rot_y = input.mouse_rot_y;
            if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
                mouse_rot_y += GetMouseDelta().x;
            }
            // TODO: actually aggregate these inputs, don't just sample and override
            input = read_movement_input();
            input.mouse_rot_y = mouse_rot_y;
        }
     );
}

inline void register_movement_input_buffer_system(
        flecs::world& world,
        InputBuffer& buffer,
        uint16_t& tick,
        flecs::timer timer)
{
    // Save the active MovementInput to the input buffer, per-tick
    world.system<MovementInput>()
        .tick_source(timer)
        .kind(flecs::OnLoad)
        .each([&] (MovementInput input) {
            buffer.push(input, tick);
        }
     );
}

inline void register_movement_input_cleanup_system(flecs::world& world, flecs::timer& timer, flecs::entity& phase) {
    // Clean up the active MovementInput at the end of each tick
    world.system<MovementInput>()
        .tick_source(timer)
        .kind(phase)
        .each([] (MovementInput& input) {
            input = {};
        }
     );
}

// MOVEMENT PROCESSING SYSTEMS
inline void register_movement_recv_system(flecs::world& world, flecs::timer& timer) {
    // Overwrite all simulated state with received state.
    // This keeps responsibilities clear, network layer does not touch simulation directly
    world.system<AckTick, RecvAckTick,
                 SimPosition, SimRotation, SimGravity, SimGrounded,
                 RecvPosition, RecvRotation, RecvGravity, RecvGrounded>()
        .tick_source(timer)
        .kind(flecs::PreUpdate)
        .each([] (
                AckTick& ack_tick,
                RecvAckTick recv_ack_tick,
                SimPosition& sim_pos,
                SimRotation& sim_rot,
                SimGravity& sim_grav,
                SimGrounded& sim_grounded,
                RecvPosition recv_pos,
                RecvRotation recv_rot,
                RecvGravity recv_grav,
                RecvGrounded recv_grounded
            )
        {
            // If we received an old tick, we don't want to go back to it
            if ((int16_t) (recv_ack_tick.val - ack_tick.val) < 0) {
                return;
            }
            sim_pos.val = recv_pos.val;
            sim_rot.val = recv_rot.val;
            sim_grav.val = recv_grav.val;
            sim_grounded.val = recv_grounded.val;
            ack_tick.val = recv_ack_tick.val;
        }
    );
}

inline void register_movement_prediction_system(flecs::world& world, InputBuffer& input_buffer, flecs::timer& timer) {
    world.system<SimPosition, SimRotation, PredPosition, PredRotation>()
        .without<LocalPlayer>()
        .tick_source(timer)
        .each([&](
                SimPosition& pos,
                SimRotation& rot,
                PredPosition& pred_pos,
                PredRotation& pred_rot
            )
        {
            // Server authoritative state becomes base for new prediction
            pred_pos.val = pos.val;
            pred_rot.val = rot.val;
        }
    );
    world.system<SimPosition, SimRotation, SimGravity, SimGrounded,
                 PredPosition, PredRotation, PredGravity, PredGrounded,
                 AckTick>()
        .with<LocalPlayer>()
        .tick_source(timer)
        .each([&](
                SimPosition& pos,
                SimRotation& rot,
                SimGravity& gravity,
                SimGrounded& grounded,
                PredPosition& pred_pos,
                PredRotation& pred_rot,
                PredGravity& pred_gravity,
                PredGrounded& pred_grounded,
                AckTick& ack_tick
            )
        {
#ifndef DISABLE_SERVER
            // Server authoritative state becomes base for new prediction
            pred_pos.val = pos.val;
            pred_rot.val = rot.val;
            pred_gravity.val = gravity.val;
            pred_grounded.val = grounded.val;
            // If new tick, perform client-side prediction on un-acked inputs
            input_buffer.flushUpTo(ack_tick.val);
            for (int i = 0; i < input_buffer.size; i++) {
                MovementInput input = input_buffer.get_at(i);
                tick_movement(
                    world,
                    input,
                    pred_pos.val,
                    pred_rot.val.y,
                    pred_gravity.val,
                    pred_grounded.val
                );
            }
#endif
        }
    );
}

inline void register_movement_transmit_system(
    flecs::world& world,
    Network& network,
    InputBuffer& input_buffer,
    uint16_t& tick,
    flecs::timer& timer)
{
    world.system()
        .with<LocalPlayer>()
        .tick_source(timer)
        .kind(flecs::PostUpdate)
        .each([&network, &input_buffer, &tick]() {
            MovementInputPacket pkt;
            pkt.tick = tick;
            pkt.size = input_buffer.size;
            input_buffer.copy_to_array(pkt.inputs);
            auto [buffer, size] = serialize(pkt);
            network.queue_data_unreliable(buffer, size);
        }
    );
}

inline void register_movement_tick_system(flecs::world& world, uint16_t& movement_tick, flecs::timer& timer) {
    world.system()
        .tick_source(timer)
        .kind(flecs::OnLoad)
        .each([&movement_tick]() {
            movement_tick++;
        }
    );
}

// MOVEMENT LERP SYSTEMS

inline void register_movement_lerp_reset_system(flecs::world& world, flecs::timer& timer) {
    // Slide over prev pointers to entity's current render state.
    world.system<RenderPosition, PrevPredPosition, RenderRotation, PrevPredRotation, LerpTimer>()
        .tick_source(timer)
        .each([] (RenderPosition& cur_pos, PrevPredPosition& prev_pos, RenderRotation& cur_rot, PrevPredRotation& prev_rot, LerpTimer& timer) {
            timer.val = 0;
            prev_pos.val = cur_pos.val;
            prev_rot.val = cur_rot.val;
            }
        );
}

inline void register_movement_lerp_system(flecs::world& world) {
    // Interpolate predicted positions between movement ticks
    world.system<RenderPosition, PredPosition, PrevPredPosition,
        RenderRotation, PredRotation, PrevPredRotation,
        LerpTimer>()
        .each([] (
                RenderPosition& pos,
                const PredPosition& target_pos,
                const PrevPredPosition& prev_pos,
                RenderRotation& rot,
                const PredRotation& target_rot,
                const PrevPredRotation& prev_rot,
                LerpTimer& timer)
        {
            float dt = GetFrameTime();
            timer.val += dt;
            float ratio = timer.val / MOVE_UPDATE_RATE;
            if (ratio > 1.0) {
                ratio = 1.0;
            }
            pos.val = Vector3Lerp(prev_pos.val, target_pos.val, ratio);
            rot.val.y = angle_slerp(prev_rot.val.y, target_rot.val.y, ratio);
        }
    );
}

