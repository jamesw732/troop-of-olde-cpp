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


inline void register_movement_reconcile_system(flecs::world& world, InputBuffer& input_buffer) {
    world.system<SimPosition, SimRotation, SimGravity, SimGrounded,
                 PredPosition, PredRotation, PredGravity, PredGrounded,
                 AckTick, LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([&input_buffer, &world](
                SimPosition& pos,
                SimRotation& rot,
                SimGravity& gravity,
                SimGrounded& grounded,
                PredPosition& pred_pos,
                PredRotation& pred_rot,
                PredGravity& pred_gravity,
                PredGrounded& pred_grounded,
                AckTick& new_ack_tick,
                LocalPlayer)
        {
            // If old tick, skip reconciliation
            if ((int16_t) (new_ack_tick.val - input_buffer.ack_tick) <= 0) {
                return;
            }
            // Server authoritative state becomes base for new prediction
            pred_pos.val = pos.val;
            pred_rot.val = rot.val;
            pred_gravity.val = gravity.val;
            pred_grounded.val = grounded.val;
            // If new tick, perform client-side reconciliation
            input_buffer.flushUpTo(new_ack_tick.val);
#ifndef DISABLE_SERVER
            for (MovementInput input: input_buffer.buffer) {
                tick_movement(world, pos.val, rot.val.y, input, gravity.val, grounded.val);
            }
#endif
        }
    );
}

inline void register_movement_input_system(
        flecs::world& world,
        InputHandler& input_handler,
        InputBuffer& input_buffer)
{
    world.system()
        .interval(MOVE_UPDATE_RATE)
        .each([&input_handler, &input_buffer]() {
            input_buffer.push(input_handler.get_movement_input());
        }
    );
}

inline void register_movement_system(
        flecs::world& world,
        InputBuffer& input_buffer)
{
    world.system<PredPosition, PredRotation, SimGravity, SimGrounded, LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([&input_buffer, &world]
           (PredPosition& pos,
            PredRotation& rot,
            SimGravity& gravity,
            SimGrounded& grounded,
            LocalPlayer)
        {
            tick_movement(world, pos.val, rot.val.y, input_buffer.back(), gravity.val, grounded.val);
        }
    );
}

inline void register_movement_transmit_system(flecs::world& world, Network& network, InputBuffer& input_buffer,
        uint16_t& tick) {
    world.system()
        .interval(MOVE_UPDATE_RATE)
        .each([&network, &input_buffer, &tick]() {
            MovementInputPacket input_data;
            input_data.tick = tick;
            for (MovementInput input: input_buffer.buffer) {
                input_data.inputs.push_back(input);
            }
            // TODO: test this in a high-latency setting
            if (input_buffer.buffer.size() >= MAX_INPUT_BUFFER) {
                int diff = input_buffer.buffer.size() - MAX_INPUT_BUFFER;
                input_buffer.buffer.erase(input_buffer.buffer.begin(),
                    input_buffer.buffer.begin() + diff + 1
                );
            }
            auto [buffer, size] = serialize(input_data);
            network.queue_data_unreliable(buffer, size);

            // Test serialize
            // InputPacket res;
            // auto state = bitsery::quickDeserialization(
            //     InputAdapter{
            //         data_buffer.data(),
            //         size
            //     },
            //     res
            // );
            // std::cout << "Sending inputs: ";
            // for (MovementInput input: res.inputs) {
            //     std::cout << (int) input.x << ", " << (int) input.z << " ";
            // }
            // std::cout << std::endl;
            }
        );
}

inline void register_movement_tick_system(flecs::world& world, uint16_t& movement_tick) {
    world.system()
        .interval(MOVE_UPDATE_RATE)
        .each([&movement_tick]() {
                movement_tick++;
            }
        );
}

inline void register_movement_lerp_reset_system(flecs::world& world) {
    world.system<RenderPosition, PrevPredPosition, RenderRotation, PrevPredRotation, LerpTimer>()
        .interval(MOVE_UPDATE_RATE)
        .each([] (RenderPosition& cur_pos, PrevPredPosition& prev_pos, RenderRotation& cur_rot, PrevPredRotation& prev_rot, LerpTimer& timer) {
            timer.val = 0;
            prev_pos.val = cur_pos.val;
            prev_rot.val = cur_rot.val;
            }
        );
}

inline void register_movement_lerp_system(flecs::world& world) {
    world.system<RenderPosition, PredPosition, PrevPredPosition,
        RenderRotation, PredRotation, PrevPredRotation,
        LerpTimer>()
        .each([] (
                RenderPosition& pos,
                PredPosition& target_pos,
                PrevPredPosition& prev_pos,
                RenderRotation& rot,
                PredRotation& target_rot,
                PrevPredRotation& prev_rot,
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

