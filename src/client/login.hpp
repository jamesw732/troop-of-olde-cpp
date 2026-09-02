#pragma once
#include <unordered_map>
#include <vector>

#include "flecs.h"

#include "components.hpp"
#include "entities.hpp"
#include "network.hpp"
#include "../shared/network-components.hpp"
#include "../mapgen/mapgen-util.hpp"
#include "../shared/packets.hpp"
#include "../shared/components.hpp"

struct LoginHandler {
    flecs::world& world;
    std::unordered_map<std::string, ModelAsset>& loaded_models;
    std::unordered_map<uint32_t, flecs::entity>& netid_to_entity;
    Map& map;
    std::vector<LoginResponsePacket> logins;
    std::vector<PlayerSpawnPacket> spawns;

    void handle_logins() {
        // there should just be one login response packet
        for (LoginResponsePacket pkt: logins) {
            std::cout << "Received login packet\n";
            for (PlayerSpawnState spawn_state: pkt.spawn_states) {
                flecs::entity entity;
                if (spawn_state.network_id == pkt.local_player_id) {
                    entity = create_local_player(world);
                    entity.set<CamRotation>({30});
                }
                else {
                    entity = create_remote_player(world);
                }
                apply_spawn_state(entity, spawn_state);
                map = pkt.map;
                create_dungeon();
                // TODO: Use base location somehow
            }
            // std::cout << "Batch Spawn Packet: " << '\n';
            // for (auto pair: netid_to_entity) {
            //     std::cout << (int) pair.first.id << ", " << (int) pair.second << '\n';
            // }
        }
        logins.clear();

        for (PlayerSpawnPacket pkt: spawns) {
            PlayerSpawnState spawn_state = pkt.spawn_state;
            flecs::entity entity = create_remote_player(world);
            apply_spawn_state(entity, spawn_state);
            // std::cout << "Single Spawn Packet: " << '\n';
            // std::cout << spawn_state.network_id.id << '\n';
            // for (auto pair: netid_to_entity) {
            //     std::cout <<  pair.first.id << ", " << pair.second << '\n';
            // }
        }
        spawns.clear();
    }

    void apply_spawn_state(flecs::entity entity, PlayerSpawnState spawn_state) {
        entity.set<NetworkId>({spawn_state.network_id});
        entity.set<PredPosition>({spawn_state.pos});
        entity.set<PredRotation>({spawn_state.rot});
        entity.set<PrevPredPosition>({spawn_state.pos});
        entity.set<SimPosition>({spawn_state.pos});
        entity.set<PrevPredRotation>({spawn_state.rot});
        entity.set<RenderPosition>({spawn_state.pos});
        entity.set<RenderRotation>({spawn_state.rot});
        entity.set<SimRotation>({spawn_state.rot});
        entity.set<DisplayName>({spawn_state.name});
        entity.set<Color>(ORANGE);
        // Hardcode model
        entity.set<ModelPointer>({&loaded_models.at("paladin").model});
        entity.set<ModelAnimations>({&loaded_models.at("paladin").animations});
        netid_to_entity[spawn_state.network_id] = entity;
    }

    void create_dungeon() {
        for (int row = 0; row < map.rows; row++) {
            for (int col = 0; col < map.cols; col++) {
                MapCell cell = map.get({col, row});
                std::string room_name = "room_" + std::to_string((int) cell.direction);
                auto room = world.entity();
                room.set<Color>(GRAY);
                room.set<ModelPointer>({&loaded_models[room_name].model});
                room.add<Scale>();
                room.add<SimPosition>();
                room.add<SimRotation>();
                room.add<RenderPosition>();
                room.add<RenderRotation>();
                room.add<Terrain>();
                room.set<Scale>({{1, 1, 1}});
                Vector3 position{static_cast<float>(col * ROOM_SIZE), 0, static_cast<float>(row * ROOM_SIZE)};
                room.set<SimPosition>({position});
                room.set<RenderPosition>({position});
            }
        }
    }
};
