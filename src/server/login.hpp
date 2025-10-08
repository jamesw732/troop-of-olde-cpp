#include "enet.h"
#include "flecs.h"

#include "server/components.hpp"
#include "shared/packets.hpp"
#include "shared/serialize.hpp"
#include "server/network.hpp"


inline void register_login_system(flecs::world& world) {
    world.system<ClientLoginPacket>()
        .each([&world] (flecs::entity e, ClientLoginPacket& packet) {
            // std::cout << "Performing login system" << std::endl;
            e.set<DisplayName>(packet.name);
            }
        );
}

inline void register_batch_spawn_system(flecs::world& world) {
    world.system<Connection, NetworkId>()
        .with<ClientLoginPacket>()
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
                e.remove<ClientLoginPacket>();
            }
        );
}
