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
    world.system<SimPosition, SimRotation, Gravity, Grounded, ClientMoveTick, MovementInputPacket,
        PredPosition, PredRotation, PredGravity, PredGrounded>()
        .interval(MOVE_UPDATE_RATE)
        .each([&world]
           (SimPosition& pos,
            SimRotation& rot,
            Gravity& gravity,
            Grounded& grounded,
            ClientMoveTick& ack_tick,
            MovementInputPacket& packet,
            PredPosition& pred_pos,
            PredRotation& pred_rot,
            PredGravity& pred_gravity,
            PredGrounded& pred_grounded
            )
        {
            uint16_t start_tick = packet.tick - (packet.inputs.size() - 1);
            int const dif = (int) (ack_tick.val - start_tick);
            // Precondition: start tick is at most 1 more than stored tick
            // If not satisfied, process whole buffer and hope for the best
            int start_idx = std::max(0, dif + 1);
            for (auto it = packet.inputs.begin() + start_idx; it != packet.inputs.end(); ++it)
            {
                auto input = *it;
                tick_movement(world, pos.val, rot.val.y, input, gravity.val, grounded.val);
            }
            if (ack_tick.val < packet.tick) {
                pred_pos.val = pos.val;
                pred_rot.val = rot.val;
                pred_gravity.val = gravity.val;
                pred_grounded.val = grounded.val;
            }
            ack_tick.val = packet.tick;
        }
    );
}

inline void register_movement_networking_system(flecs::world& world, Network& network) {
    world.system<NetworkId, ClientMoveTick, SimPosition, SimRotation, Gravity, Grounded>()
        .with<Connected>()
        .interval(MOVE_UPDATE_RATE)
        .each([&]
             (NetworkId& network_id,
              ClientMoveTick& ack_tick,
              SimPosition& pos,
              SimRotation& rot,
              Gravity& gravity,
              Grounded& grounded)
        {
            MovementUpdateBatchPacket batch;
            batch.move_updates.clear();
            MovementUpdate move_update{network_id.id, ack_tick.val, pos.val, rot.val, gravity.val, grounded.val};
            // Send simulated position for remote clients, only send data for inputs we received for local player
            batch.move_updates.push_back(move_update);
            world.query<NetworkId, ClientMoveTick, PredPosition, PredRotation, PredGravity, PredGrounded>()
                .each([&]
                    (NetworkId& remote_network_id,
                     ClientMoveTick& remote_ack_tick,
                     PredPosition& pred_pos,
                     PredRotation& pred_rot,
                     PredGravity& pred_gravity,
                     PredGrounded& pred_grounded)
                {
                    MovementUpdate remote_move_update{
                        remote_network_id.id,
                        remote_ack_tick.val,
                        pred_pos.val,
                        pred_rot.val,
                        pred_gravity.val,
                        pred_grounded.val
                    };
                    batch.move_updates.push_back(remote_move_update);
                }
            );
            auto [buffer, size] = serialize(batch);
            network.queue_data_unreliable(network_id, buffer, size);
        }
    );
}

inline void register_movement_prediction_system(flecs::world& world) {
    world.system<PredPosition, PredRotation, PredGravity, PredGrounded>()
        .interval(MOVE_UPDATE_RATE)
        .each([&world]
           (PredPosition& pos,
            PredRotation& rot,
            PredGravity& gravity,
            PredGrounded& grounded
            )
        {
            // TODO: Consider predicting with most recent movement input
            // Also consider predicting client-side for remote players
            tick_movement(world, pos.val, rot.val.y, MovementInput{0, 0, 0, 0, 0}, gravity.val, grounded.val);
        }
    );
}
