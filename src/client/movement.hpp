#pragma once
#include <unordered_map>

#include "camera.hpp"
#include "flecs.h"
#include "raylib-cpp.hpp"

#include "input.hpp"
#include "components.hpp"
#include "network.hpp"
#include "../shared/movement.hpp"
#include "../shared/components.hpp"
#include "../shared/const.hpp"
#include "../shared/serialize.hpp"
#include "../shared/util.hpp"
#include "../shared/raylib-util.hpp"


inline void register_movement_recv_system(flecs::world& world) {
    world.system<MovementUpdateBatchPacket>()
        .interval(MOVE_UPDATE_RATE)
        .each([&] (MovementUpdateBatchPacket& batch) {
            // TODO: introduce more lag for remote players to smooth movement
            auto& netid_to_entity = world.get_mut<NetworkMap>().netid_to_entity;
            for (MovementUpdate move_update: batch.move_updates) {
                auto netid_entity = netid_to_entity.find(move_update.network_id);
                if (netid_entity == netid_to_entity.end()) {
                    continue;
                }
                flecs::entity e = netid_entity->second;
                if ((int16_t) (move_update.ack_tick.val - e.get<AckTick>().val) <= 0) {
                    continue;
                }
                e.set<AckTick>({move_update.ack_tick.val});
                e.set<SimPosition>({move_update.pos.val});
                e.set<SimRotation>({move_update.rot.val});
            }
        }
    );
}

inline void register_movement_reconcile_system(flecs::world& world, InputBuffer& input_buffer) {
    world.system<SimPosition, SimRotation, AckTick, LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([&input_buffer](
                SimPosition& pos,
                SimRotation& rot,
                AckTick& new_ack_tick,
                LocalPlayer
            ) {
            // If old tick, skip reconciliation
            if ((int16_t) (new_ack_tick.val - input_buffer.ack_tick) <= 0) {
                return;
            }
            // If new tick, perform client-side reconciliation
            input_buffer.flushUpTo(new_ack_tick.val);
            for (MovementInput input: input_buffer.buffer) {
                raylib::Vector3 disp = process_movement_input(input, rot.val.y);
                pos.val += disp;
            }
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
    world.system<SimPosition, SimRotation, LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([&input_buffer](SimPosition& pos, SimRotation& rot, LocalPlayer) {
            raylib::Vector3 disp = process_movement_input(input_buffer.back(), rot.val.y);
            pos.val += disp;
        }
    );
}

inline void register_movement_transmit_system(flecs::world& world, Network& network, InputBuffer& input_buffer,
        uint16_t& tick) {
    world.system()
        .interval(MOVE_UPDATE_RATE)
        .each([&network, &input_buffer, &tick]() {
            // Construct movement input packet from input buffer and send to server
            MovementInputPacket input_data;
            input_data.tick = tick;
            for (MovementInput input: input_buffer.buffer) {
                input_data.inputs.push_back(input);
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
    // Slide over current position to prev. Target will be mutated by other systems.
    world.system<RenderPosition, PrevSimPosition, RenderRotation, PrevSimRotation, LerpTimer>()
        .interval(MOVE_UPDATE_RATE)
        .each([] (RenderPosition& cur_pos, PrevSimPosition& prev_pos, RenderRotation& cur_rot, PrevSimRotation& prev_rot, LerpTimer& timer) {
            timer.val = 0;
            prev_pos.val = cur_pos.val;
            prev_rot.val = cur_rot.val;
            }
        );
}

inline void register_movement_lerp_system(flecs::world& world) {
    world.system<RenderPosition, SimPosition, PrevSimPosition,
        RenderRotation, SimRotation, PrevSimRotation,
        LerpTimer>()
        .each([] (
                RenderPosition& pos,
                SimPosition& target_pos,
                PrevSimPosition& prev_pos,
                RenderRotation& rot,
                SimRotation& target_rot,
                PrevSimRotation& prev_rot,
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

