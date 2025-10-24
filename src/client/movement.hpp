#pragma once
#include <unordered_map>

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


inline void register_movement_target_system(flecs::world& world) {
    // Slide over current position to prev. Target will be mutated by other systems.
    world.system<Position, PrevPosition, LerpTimer>()
        .interval(MOVE_UPDATE_RATE)
        .each([] (Position& target_pos, PrevPosition& prev_pos, LerpTimer& timer) {
            timer.val = 0;
            prev_pos.val = target_pos.val;
            // std::cout << "Previous position: " << vector3_to_string(prev_pos.val) << std::endl;
            }
        );
}

inline void register_movement_recv_system(flecs::world& world) {
    world.system<MovementUpdateBatchPacket>()
        .interval(MOVE_UPDATE_RATE)
        .each([&] (MovementUpdateBatchPacket& batch) {
            // dbg("Running movement batch system");
            auto& netid_to_entity = world.get_mut<NetworkMap>().netid_to_entity;
            for (MovementUpdate move_update: batch.move_updates) {
                auto netid_entity = netid_to_entity.find(move_update.network_id);
                if (netid_entity == netid_to_entity.end()) {
                    continue;
                }
                flecs::entity e = netid_entity->second;
                // dbg(move_update.ack_tick.val);
                // std::cout << move_update.ack_tick.val << ", " << e.get<AckTick>().val << '\n';
                if ((int16_t) (move_update.ack_tick.val - e.get<AckTick>().val) <= 0) {
                    // std::cout << "Skipping entity" << '\n';
                    continue;
                }
                e.set<AckTick>({move_update.ack_tick.val});
                e.set<TargetPosition>({move_update.pos.val});
            }
        }
    );
}

inline void register_movement_reconcile_system(flecs::world& world, InputBuffer& input_buffer) {
    world.system<TargetPosition, AckTick, LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([&input_buffer](
                TargetPosition& target_pos,
                AckTick& new_ack_tick,
                LocalPlayer
            ) {
            // If old tick, skip reconciliation
            if ((int16_t) (new_ack_tick.val - input_buffer.ack_tick) <= 0) {
                // std::cout << "Skipping client-side reconciliation" << std::endl;
                return;
            }
            // If new tick, perform client-side reconciliation
            input_buffer.flushUpTo(new_ack_tick.val);
            for (MovementInput input: input_buffer.buffer) {
                // std::cout << "Processing movement: " << (int) input.x << ", " << (int) input.z << std::endl;
                process_movement_input(target_pos.val, input);
            }
        }
    );
}

inline void register_movement_input_system(
        flecs::world& world,
        InputHandler& input_handler,
        InputBuffer& input_buffer
        ) {
    world.system<MovementInput, LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([&input_handler, &input_buffer](MovementInput& input, LocalPlayer) {
                input = input_handler.get_movement_input();
                input_buffer.push(input);
            }
        );
}

inline void register_movement_system(flecs::world& world) {
    world.system<TargetPosition, MovementInput, LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([](TargetPosition& pos, MovementInput& input, LocalPlayer) {
                process_movement_input(pos.val, input);
                // dbg("Target Position", pos.val);
                // std::cout << "Processing movement: " << (int) input.x << ", " << (int) input.z << std::endl;
                // std::cout << "Target position: " << vector3_to_string(pos.val) << std::endl;
            }
        );
}

inline void register_movement_networking_system(flecs::world& world, Network& network, InputBuffer& input_buffer,
        uint16_t& tick) {
    world.system<LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([&network, &input_buffer, &tick](LocalPlayer) {
            // Construct movement input packet from input buffer and send to server
            MovementInputPacket input_data;
            input_data.tick = tick;
            for (MovementInput input: input_buffer.buffer) {
                // std::cout << (int) input.x << ", " << (int) input.z << std::endl;
                input_data.inputs.push_back(input);
            }
            auto [buffer, size] = serialize(input_data);
            network.queue_data_reliable(buffer, size);

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
    world.system<LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([&movement_tick](LocalPlayer) {
                movement_tick++;
                // std::cout << "Begin tick " << (int) movement_tick << std::endl;
            }
        );
}

inline flecs::system register_movement_lerp_system(flecs::world& world, float& dt) {
    return world.system<Position, TargetPosition, PrevPosition, LerpTimer>()
        .each([&dt] (Position& pos, TargetPosition& target_pos, PrevPosition& prev_pos, LerpTimer& timer) {
                timer.val += dt;
                // std::cout << timer.val << std::endl;
                float ratio = timer.val / MOVE_UPDATE_RATE;
                // std::cout << timer.val << ", " << ratio << std::endl;
                if (ratio > 1.0) {
                    ratio = 1.0;
                }
                pos.val = Vector3Lerp(prev_pos.val, target_pos.val, ratio);
            }
        );
}

