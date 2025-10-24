#pragma once
#include <iostream>

#include <flecs.h>

#include "components.hpp"
#include "network.hpp"
#include "../shared/packets.hpp"
#include "../shared/components.hpp"
#include "../shared/const.hpp"
#include "../shared/serialize.hpp"
#include "../shared/util.hpp"


inline void register_movement_system(flecs::world& world) {
    world.system<Position, ClientMoveTick, MovementInputPacket>()
        .interval(MOVE_UPDATE_RATE)
        .each([](Position& pos, ClientMoveTick& ack_tick, MovementInputPacket& packet) {
            uint16_t start_tick = packet.tick - (packet.inputs.size() - 1);
            // std::cout << "Processing " << packet.tick - ack_tick.val << " movement inputs" << std::endl;
            int const dif = (int) (ack_tick.val - start_tick);
            // Precondition: received tick is at most 1 more than stored tick
            // If not satisfied, process whole buffer and hope for the best
            int start_idx = std::max(0, dif + 1);
            for (auto it = packet.inputs.begin() + start_idx;
                    it != packet.inputs.end(); ++it) {
                auto input = *it;
                raylib::Vector3 velocity((float) input.x, 0, (float) input.z);
                velocity = velocity.Normalize();
                velocity = velocity * 0.25;
                pos.val += velocity;
            }
            ack_tick.val = packet.tick;
        }
    );
}

inline void register_movement_batch_system(flecs::world& world) {
    world.system<NetworkId, ClientMoveTick, Position>()
        .with<Connected>()
        .interval(MOVE_UPDATE_RATE)
        .each([&] (
                  NetworkId& network_id,
                  ClientMoveTick& ack_tick,
                  Position& pos) {
            // dbg("Batching movement updates");
            MovementUpdateBatchPacket & batch = world.get_mut<MovementUpdateBatchPacket>();
            MovementUpdate move_update{network_id, ack_tick, pos};
            batch.move_updates.push_back(move_update);
        }
    );
}

inline void register_movement_batch_networking_system(flecs::world& world, Network& network) {
    world.system<NetworkId>()
        .with<Connected>()
        .interval(MOVE_UPDATE_RATE)
        .each([&] (const NetworkId& network_id) {
            // dbg("Sending batched movement updates");
            const auto& batch = world.get<MovementUpdateBatchPacket>();
            // Serialize position
            auto [buffer, size] = serialize(batch);
            // Create packet and send to client
            network.queue_data_unreliable(network_id, buffer, size);
        }
    );
}

inline void register_movement_batch_clear_system(flecs::world& world) {
    world.system<MovementUpdateBatchPacket>()
        .interval(MOVE_UPDATE_RATE)
        .each([&] (MovementUpdateBatchPacket& batch) {
            batch.move_updates.clear();
        }
    );
}
