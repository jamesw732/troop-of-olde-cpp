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


/*
 * Process movement inputs from a MovementInputPacket and reset local prediction if new packet.
 *
 * TODO: Consider adding a "Predicting" component for characters, to refactor predictions
 */
inline void register_movement_system(flecs::world& world) {
    world.system<SimPosition, SimRotation, SimGravity, SimGrounded, ClientMoveTick, MovementInputPacket,
        PredPosition, PredRotation, PredGravity, PredGrounded>()
        .interval(MOVE_UPDATE_RATE)
        .each([&world]
           (SimPosition& pos,
            SimRotation& rot,
            SimGravity& gravity,
            SimGrounded& grounded,
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

/*
 * Sends the current state of the world to all clients.
 * State differs between clients, local player has the actual simulated state, but
 * remote players have their predicted state.
 */
inline void register_movement_networking_system(flecs::world& world, Network& network) {
    world.system<NetworkId, ClientMoveTick, SimPosition, SimRotation, SimGravity, SimGrounded>()
        .with<Connected>()
        .interval(MOVE_UPDATE_RATE)
        .each([&]
             (NetworkId& network_id,
              ClientMoveTick& ack_tick,
              SimPosition& pos,
              SimRotation& rot,
              SimGravity& gravity,
              SimGrounded& grounded)
        {
            MovementUpdateBatchPacket batch;
            batch.move_updates.clear();
            MovementUpdate move_update{
                network_id.id,
                ack_tick.val,
                pos.val,
                rot.val,
                gravity.val,
                grounded.val
           };
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
                    if (remote_network_id.id == network_id.id) {
                        return;
                    }
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

/*
 * Updates a temporary prediction of each player's state by assuming a zero MovementInput
 *
 * TODO: Consider predicting with most recent movement input
 * Also consider predicting client-side for remote players
 * Also consider making this not so reliant on system order. If this goes between
 * the normal movement system and movement networking system, the advertised state will be wrong.
 * Is it possible to make this idempotent?
 */
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
            tick_movement(
                world,
                pos.val,
                rot.val.y,
                MovementInput{},
                gravity.val,
                grounded.val
            );
        }
    );
}
