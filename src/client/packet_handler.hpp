#pragma once
#include "entities.hpp"
#include "login.hpp"
#include "network.hpp"
#include "../shared/packets.hpp"
#include "../shared/serialize.hpp"

/*
 * This struct acts as a shallow boundary between networking data and ECS data
 * Game logic should be extremely minimal in this file
 * The desired pattern is to store received networking data in "intermediary" ECS components,
 * to be processed later in a system like any other component would.
 */
struct PacketHandler {
    flecs::world& world;
    LoginHandler& login_handler;
    std::unordered_map<std::string, ModelAsset>& loaded_models;
    std::unordered_map<uint32_t, flecs::entity>& netid_to_entity;

    void handle_packets(std::vector<std::vector<uint8_t>>& packets) {
        for (std::vector<uint8_t> packet: packets) {
            handle_packet(packet);
        }
        packets.clear();
    }

    void handle_packet(std::vector<uint8_t>& packet_data) {
        // This function should not do any complicated operations, those should be done by systems
        // Tentative heuristic is that if we need to query for a component, it's too complicated
        bitsery::Deserializer<InputAdapter> des{InputAdapter{packet_data.data(), packet_data.size()}};
        PacketType pkt_type;
        des.value1b(pkt_type);
        switch (pkt_type) {
            case PacketType::LoginResponsePacket: {
                // We just entered world and need world's whole state
                LoginResponsePacket login_response;
                des.object(login_response);
                login_handler.logins.push_back(login_response);
                break;
            }

            case PacketType::PlayerSpawnPacket: {
                // Remote player entered world and we need that new player's state
                PlayerSpawnPacket spawn_packet;
                des.object(spawn_packet);
                login_handler.spawns.push_back(spawn_packet);
                break;
            }

            case PacketType::MovementUpdateBatchPacket: {
                MovementUpdateBatchPacket batch;
                des.object(batch);
#ifndef DISABLE_SERVER
                for (MovementUpdate move_update: batch.move_updates) {
                    /* if (!netid_to_entity.contains(move_update.network_id)) { */
                    /*     continue; */
                    /* } */
                    auto netid_entity = netid_to_entity.find(move_update.network_id);
                    if (netid_entity == netid_to_entity.end()) {
                        continue;
                    }
                    flecs::entity e = netid_entity->second;
                    /* if ((int16_t) (move_update.ack_tick - e.get<AckTick>().val) <= 0) { */
                    /*     continue; */
                   /* } */
                    e.set<RecvAckTick>({move_update.ack_tick});
                    e.set<RecvPosition>({move_update.pos});
                    e.set<RecvRotation>({move_update.rot});
                    e.set<RecvGravity>({move_update.gravity});
                    e.set<RecvGrounded>({move_update.grounded});
                    e.set<RecvLocomotionBlendSpace>({move_update.blend_space});
                }
#endif
                break;
            }

            case PacketType::DisconnectPacket: {
                DisconnectPacket dc_packet;
                des.object(dc_packet);
                std::cout << dc_packet.client_id << '\n';
                auto netid_entity = netid_to_entity.find(dc_packet.client_id);
                if (netid_entity == netid_to_entity.end()) {
                    break;
                }
                flecs::entity entity = netid_entity->second;
                entity.add<Disconnected>();
                break;
            }

            default: {
                break;
            }
        }
    }
};
