#include "enet.h"
#include "flecs.h"

#include "server/components.hpp"
#include "server/network.hpp"
#include "shared/packets.hpp"
#include "shared/serialize.hpp"


inline void register_batch_spawn_system(flecs::world& world) {
    /*
     * Batches world state and sends to new client
     */
    world.system<Connection, NetworkId>()
        .with<NeedsSpawnBatch>()
        .each([&world] (flecs::entity e, Connection& conn, NetworkId& local_player_id) {
            // std::cout << "Making spawn batch" << std::endl;
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
            auto [buffer, size] = serialize(spawns);
            ENetPacket* packet = enet_packet_create(buffer.data(), size, ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(conn.peer, 1, packet);
            // Remove login packet from entity
            e.remove<NeedsSpawnBatch>();
        }
    );
}

inline void register_spawn_broadcast_system(flecs::world& world) {
    /*
     * Sends new character information to all existing clients
     */
    world.system<NetworkId, DisplayName, Position>()
        .with<NeedsSpawnBroadcast>()
        .each([&world] (flecs::entity e,
                        const NetworkId& network_id,
                        const DisplayName& name,
                        const Position& pos
            ) {
            // std::cout << "Broadcasting spawn" << '\n';
            PlayerSpawnPacket spawn_packet{{network_id, name, pos}};
            world.query<Connection, NetworkId>()
                .each([&] (const Connection& conn, const NetworkId& tgt_network_id) {
                    if (network_id.id == tgt_network_id.id) {
                        // std::cout << "Skipping client with network id " << tgt_network_id.id << '\n';
                        return;
                    }
                    // std::cout << "Sending character with network id " << network_id.id
                    // << " to client with network id " << tgt_network_id.id
                    // << '\n';
                    auto [buffer, size] = serialize(spawn_packet);
                    ENetPacket* packet = enet_packet_create(buffer.data(), size, ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(conn.peer, 1, packet);
                }
            );
            e.remove<NeedsSpawnBroadcast>();
        }
    );
}
