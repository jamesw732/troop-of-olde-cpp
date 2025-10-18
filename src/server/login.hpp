#include "flecs.h"

#include "server/components.hpp"
#include "server/network.hpp"
#include "shared/packets.hpp"
#include "shared/serialize.hpp"


inline void register_batch_spawn_system(flecs::world& world, Network& network) {
    /*
     * Batches world state and sends to new client
     */
    world.system<NetworkId>()
        .with<NeedsSpawnBatch>()
        .with<Connected>()
        .each([&world, &network] (flecs::entity e, NetworkId& local_player_id) {
            std::vector<PlayerSpawnState> spawn_states;
            auto q = world.query<NetworkId, DisplayName, Position>();
            q.each([&spawn_states] (NetworkId& id, DisplayName& name, Position& pos) {
                    // std::cout << "Adding player to spawn_states" << std::endl;
                    spawn_states.push_back(PlayerSpawnState{id, name, pos});
                    }
                );
            SpawnBatchPacket spawns{local_player_id, spawn_states};
            // for (PlayerSpawnState spawn_state: spawns.spawn_states) {
            //     std::cout << spawn_state.name.name << std::endl;
            //     std::cout << spawn_state.network_id.id << std::endl;
            //     std::cout << vector3_to_string(spawn_state.pos.val) << std::endl;
            // }
            dbg("Sending spawn packet", local_player_id.id);
            auto [buffer, size] = serialize(spawns);
            network.queue_data_reliable(local_player_id, buffer, size);
            // Remove login packet from entity
            e.remove<NeedsSpawnBatch>();
        }
    );
}

inline void register_spawn_broadcast_system(flecs::world& world, Network& network) {
    /*
     * Sends new character information to all existing clients
     */
    world.system<NetworkId, DisplayName, Position>()
        .with<NeedsSpawnBroadcast>()
        .each([&world, &network] (flecs::entity e,
                        const NetworkId& network_id,
                        const DisplayName& name,
                        const Position& pos
            ) {
            // std::cout << "Broadcasting spawn" << '\n';
            PlayerSpawnPacket spawn_packet{{network_id, name, pos}};
            world.query<NetworkId, Connected>()
                .each([&] (const NetworkId& tgt_network_id, Connected) {
                    if (network_id.id == tgt_network_id.id) {
                        // std::cout << "Skipping client with network id " << tgt_network_id.id << '\n';
                        return;
                    }
                    // std::cout << "Sending character with network id " << network_id.id
                    // << " to client with network id " << tgt_network_id.id
                    // << '\n';
                    auto [buffer, size] = serialize(spawn_packet);
                    network.queue_data_reliable(tgt_network_id, buffer, size);
                }
            );
            e.remove<NeedsSpawnBroadcast>();
        }
    );
}
