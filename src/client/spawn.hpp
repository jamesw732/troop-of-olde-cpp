#pragma once
#include <iostream>

#include "flecs.h"

#include "client/components.hpp"
#include "shared/components.hpp"
#include "shared/packets.hpp"


inline void register_batch_spawn_system(flecs::world& world) {
    world.system<SpawnBatchPacket>()
        .each([&world] (flecs::entity e, SpawnBatchPacket& spawn_batch) {
                // std::cout << "Loading world state" << std::endl;
                NetworkId local_player_id = spawn_batch.local_player_id;
                flecs::entity iter_e;
                for (PlayerSpawnState spawn_state: spawn_batch.spawn_states) {
                    // std::cout << "Loading player" << std::endl;
                    if (spawn_state.network_id.id == local_player_id.id) {
                        iter_e = e;
                        e.add<LocalPlayer>();
                    }
                    else {
                        iter_e = world.entity();
                    }
                    iter_e.set<Position>(spawn_state.pos);
                    iter_e.set<TargetPosition>(TargetPosition{spawn_state.pos.val});
                    iter_e.set<PrevPosition>(PrevPosition{spawn_state.pos.val});
                    iter_e.add<LerpTimer>();
                    iter_e.add<MovementUpdatePacket>();
                    iter_e.add<MovementInput>();
                    iter_e.set<NetworkId>(spawn_state.network_id);
                    iter_e.set<DisplayName>(spawn_state.name);
                }
                e.remove<SpawnBatchPacket>();
            }
        );
}
