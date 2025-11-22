#pragma once
#include <iostream>

#include "flecs.h"

#include "components.hpp"
#include "network.hpp"
#include "../shared/packets.hpp"
#include "../shared/components.hpp"
#include "../shared/const.hpp"
#include "../shared/movement.hpp"
#include "../shared/serialize.hpp"
#include "../shared/util.hpp"


inline void register_movement_system(flecs::world& world) {
    world.system<SimPosition, SimRotation, Gravity, Grounded, ClientMoveTick, MovementInputPacket>()
        .interval(MOVE_UPDATE_RATE)
        .each([&world]
           (SimPosition& pos,
            SimRotation& rot,
            Gravity& gravity,
            Grounded& grounded,
            ClientMoveTick& ack_tick,
            MovementInputPacket& packet)
        {
            uint16_t start_tick = packet.tick - (packet.inputs.size() - 1);
            int const dif = (int) (ack_tick.val - start_tick);
            // Precondition: start tick is at most 1 more than stored tick
            // If not satisfied, process whole buffer and hope for the best
            int start_idx = std::max(0, dif + 1);
            // TODO: Server should simulate gravity even if no inputs were received.
            // Should do this simulation with a new component, and should only send to remote clients
            for (auto it = packet.inputs.begin() + start_idx; it != packet.inputs.end(); ++it)
            {
                auto input = *it;
                tick_movement(world, pos.val, rot.val.y, input, gravity.val, grounded.val);
            }
            ack_tick.val = packet.tick;
        }
    );
}

inline void register_movement_batch_system(flecs::world& world) {
    world.system<NetworkId, ClientMoveTick, SimPosition, SimRotation, Gravity, Grounded>()
        .with<Connected>()
        .interval(MOVE_UPDATE_RATE)
        .each([&] (
                  NetworkId& network_id,
                  ClientMoveTick& ack_tick,
                  SimPosition& pos,
                  SimRotation& rot,
                  Gravity& gravity,
                  Grounded& grounded)
        {
            MovementUpdateBatchPacket & batch = world.get_mut<MovementUpdateBatchPacket>();
            MovementUpdate move_update{network_id.id, ack_tick.val, pos.val, rot.val, gravity.val, grounded.val};
            batch.move_updates.push_back(move_update);
        }
    );
}

inline void register_movement_batch_networking_system(flecs::world& world, Network& network) {
    world.system<NetworkId>()
        .with<Connected>()
        .interval(MOVE_UPDATE_RATE)
        .each([&] (const NetworkId& network_id) {
            const auto& batch = world.get<MovementUpdateBatchPacket>();
            auto [buffer, size] = serialize(batch);
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
