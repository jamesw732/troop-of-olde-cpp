#pragma once
#include "entities.hpp"
#include "flecs.h"

#include "components.hpp"
#include "network.hpp"
#include "../shared/packets.hpp"
#include "../shared/serialize.hpp"

struct LoginRequest{
    uint32_t client_id;
    LoginRequestPacket packet;
};

struct LoginHandler {
    flecs::world& world;
    Map& map;
    Network& network;
    std::unordered_map<uint32_t, flecs::entity>& client_id_to_entity;
    std::vector<LoginRequest> logins {};

    void handle_logins() {
        for (LoginRequest login: logins) {
            // Create entity
            auto e = world.entity();
            client_id_to_entity[login.client_id] = e;
            e.set<ClientId>({login.client_id});
            add_character_components(e);
            // TODO: Generate base location
            Vector3 pos {0, 1, 0};
            Vector3 rot {0, 180, 0};
            e.set<SimPosition>({pos});
            e.set<SimRotation>({rot});
            // Send batched spawn state to local client
            std::vector<PlayerSpawnState> spawn_states;
            auto q = world.query<ClientId, DisplayName, SimPosition, SimRotation>();
            q.each([&spawn_states] (ClientId id, DisplayName name, SimPosition pos, SimRotation rot) {
                    spawn_states.push_back(PlayerSpawnState{id.id, name.name, pos.val, rot.val});
                }
            );
            LoginResponsePacket spawns{login.client_id, spawn_states, map, {0, 0}};
            auto [buffer, size] = serialize(spawns);
            network.queue_data_reliable(login.client_id, buffer, size);
            // Update remote clients with new player
            PlayerSpawnPacket spawn_packet{{login.client_id, login.packet.name, pos, rot}};
            world.query<ClientId>()
                .each([&] (const ClientId& tgt_client_id) {
                    if (login.client_id == tgt_client_id.id) {
                        // std::cout << "Skipping client with network id " << tgt_network_id.id << '\n';
                        return;
                    }
                    // std::cout << "Sending character with network id " << network_id.id
                    // << " to client with network id " << tgt_network_id.id
                    // << '\n';
                    auto [buffer, size] = serialize(spawn_packet);
                    network.queue_data_reliable(tgt_client_id.id, buffer, size);
                }
            );
        }
        logins.clear();
    }
};
