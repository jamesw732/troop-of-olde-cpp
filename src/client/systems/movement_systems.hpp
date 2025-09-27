#pragma once
#include "enet.h"
#include "flecs.h"
#include "raylib-cpp.hpp"

#include "client/input_buffer.hpp"
#include "client/input_handler.hpp"
#include "client/components/player.hpp"
#include "shared/helpers/movement.hpp"
#include "shared/components/movement.hpp"
#include "shared/components/physics.hpp"
#include "shared/components/ticks.hpp"
#include "shared/const.hpp"
#include "shared/serialize/helpers.hpp"
#include "shared/serialize/serialize_movement.hpp"


inline void register_movement_reconcile_system(flecs::world& world, InputBuffer& input_buffer) {
    auto move_rec_sys = world.system<Position, ServerMovementUpdate, LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([&input_buffer](Position& pos, ServerMovementUpdate& move_update, LocalPlayer) {
                // If old tick, skip reconciliation
                if ((int16_t) (move_update.ack_tick - input_buffer.ack_tick) <= 0) {
                    std::cout << "Skipping client-side reconciliation" << std::endl;
                    return;
                }
                // If new tick, perform client-side reconciliation
                std::cout << "Performing client-side reconciliation" << std::endl;
                input_buffer.flushUpTo(move_update.ack_tick);
                Position new_pos{move_update.pos};
                for (MovementInput input: input_buffer.buffer) {
                    std::cout << "Processing movement: " << (int) input.x << ", " << (int) input.z << std::endl;
                    process_movement_input(new_pos, input);
                }
                pos.val = new_pos.val;
            }
        );
}

inline void register_movement_input_system(
        flecs::world& world,
        InputHandler& input_handler,
        InputBuffer& input_buffer
        ) {
    auto move_input_sys = world.system<MovementInput, LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([&input_handler, &input_buffer](MovementInput& input, LocalPlayer) {
                input = input_handler.get_movement_input();
                input_buffer.push(input);
            }
        );
}

inline void register_movement_system(flecs::world& world) {
    auto move_sys = world.system<Position, MovementInput, LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([](Position& pos, MovementInput& input, LocalPlayer) {
                process_movement_input(pos, input);
            }
        );
}

inline void register_movement_networking_system(flecs::world& world, ENetPeer* peer, InputBuffer& input_buffer,
        uint16_t& tick) {
    auto move_networking_sys = world.system()
        .interval(MOVE_UPDATE_RATE)
        .each([peer, &input_buffer, &tick]() {
            // Construct movement input packet from input buffer and send to server
            MovementInputPacket input_data;
            input_data.tick = tick;
            for (MovementInput input: input_buffer.buffer) {
                // std::cout << (int) input.x << ", " << (int) input.z << std::endl;
                input_data.inputs.push_back(input);
            }
            Buffer data_buffer;
            size_t size = bitsery::quickSerialization(OutputAdapter{data_buffer}, input_data);
            ENetPacket* packet = enet_packet_create(data_buffer.data(), size, 0);
            enet_peer_send(peer, 0, packet);

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
    auto move_tick_sys = world.system()
        .interval(MOVE_UPDATE_RATE)
        .each([&movement_tick]() {
                movement_tick++;
            }
        );
}
