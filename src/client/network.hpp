#pragma once
#include <iostream>
#include <unordered_map>

#define ENET_IMPLEMENTATION
#include "enet.h"

#include "client/entities.hpp"
#include "shared/components.hpp"
#include "shared/serialize.hpp"
#include "shared/util.hpp"


class Network {
  public:
    ENetHost* client = {0};
    ENetPeer* peer = {0};
    ENetAddress address = {0};
    ENetEvent event;
    flecs::world world;
    std::unordered_map<NetworkId, flecs::entity> netid_to_entity;

    Network(flecs::world& w) : world(w) {};

    void connect() {
        connect("127.0.0.1", 7777);
    }

    void connect(std::string host_addr, int host_port) {
        client = enet_host_create(
            NULL, // Create a client host
            1, // Only allow 1 outgoing connection
            2, // Allow up to 2 channels to be used, 0 and 1
            0, // Assume any amount of incoming bandwidth
            0 // Assume any amount of outgoing bandwidth
        );
        if (client == NULL) {
            std::cout << "An error occurred while trying to create an ENet client host." << std::endl;
            exit(EXIT_FAILURE);
        }
        // Bind server address
        enet_address_set_host(&address,host_addr.c_str());
        address.port = host_port;
        // Initiate the connection, allocating the two channels 0 and 1.
        peer = enet_host_connect(client, &address, 2, 0);
        if (peer == NULL) {
            std::cout << "No available peers for initiating an ENet connection." << std::endl;
            exit(EXIT_FAILURE);
        }
        if (enet_host_service(client, &event, 10000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
            std::cout << "Connection to Troop of Olde server succeeded." << std::endl;
            ClientLoginPacket login{{"Player"}};
            auto [buffer, size] = serialize(login);
            queue_data_reliable(buffer, size);
        } else {
            enet_peer_reset(peer);
            std::cout << "Connection to Troop of Olde server failed." << std::endl;
            exit(1);
        }
    }

    void queue_data_unreliable(const Buffer& buffer, const size_t size){
        ENetPacket* packet = enet_packet_create(buffer.data(), size, 0);
        enet_peer_send(peer, 0, packet);
    }

    void queue_data_reliable(const Buffer& buffer, const size_t size){
        ENetPacket* packet = enet_packet_create(buffer.data(), size, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 1, packet);
    }

    void send_network_buffer() {
        enet_host_flush(client);
    }

    void process_events() {
        while (enet_host_service(client, &event, 0) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    // Don't do anything here, do logic in connect function
                    break;
                }
                case ENET_EVENT_TYPE_RECEIVE: {
                    handle_packet();
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT: {
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
                    break;
                }
                case ENET_EVENT_TYPE_NONE: {
                    break;
                }
            }
        }
    }

    void handle_packet() {
        uint8_t* buffer = event.packet->data;
        size_t size = event.packet->dataLength;
        bitsery::Deserializer<InputAdapter> des{InputAdapter{buffer, size}};
        PacketType pkt_type;
        des.value1b(pkt_type);
        switch (pkt_type) {
            case PacketType::SpawnBatchPacket: {
                // We just entered world
                dbg("Received batch spawn packet");
                SpawnBatchPacket spawn_batch;
                des.object(spawn_batch);
                for (PlayerSpawnState spawn_state: spawn_batch.spawn_states) {
                    flecs::entity entity;
                    if (spawn_state.network_id.id == spawn_batch.local_player_id.id) {
                        entity = create_local_player(world);
                    }
                    else {
                        entity = create_remote_player(world);
                    }
                    entity.set<Position>(spawn_state.pos);
                    entity.set<TargetPosition>(TargetPosition{spawn_state.pos.val});
                    entity.set<PrevPosition>(PrevPosition{spawn_state.pos.val});
                    entity.set<NetworkId>(spawn_state.network_id);
                    entity.set<DisplayName>(spawn_state.name);
                    netid_to_entity[spawn_state.network_id] = entity;
                }
                // std::cout << "Batch Spawn Packet: " << '\n';
                // for (auto pair: netid_to_entity) {
                //     std::cout << (int) pair.first.id << ", " << (int) pair.second << '\n';
                // }
                break;
            }

            case PacketType::PlayerSpawnPacket: {
                // Remote player entered world
                PlayerSpawnPacket spawn_packet;
                des.object(spawn_packet);
                PlayerSpawnState spawn_state = spawn_packet.spawn_state;
                flecs::entity entity = create_remote_player(world);
                entity.set<Position>(spawn_state.pos);
                entity.set<TargetPosition>(TargetPosition{spawn_state.pos.val});
                entity.set<PrevPosition>(PrevPosition{spawn_state.pos.val});
                entity.set<NetworkId>(spawn_state.network_id);
                entity.set<DisplayName>(spawn_state.name);
                netid_to_entity[spawn_state.network_id] = entity;
                dbg("Received external spawn packet");
                // std::cout << "Single Spawn Packet: " << '\n';
                // std::cout << spawn_state.network_id.id << '\n';
                // for (auto pair: netid_to_entity) {
                //     std::cout <<  pair.first.id << ", " << pair.second << '\n';
                // }
                break;
            }

            case PacketType::MovementUpdateBatchPacket: {
                MovementUpdateBatchPacket batch;
                des.object(batch);
                world.set<MovementUpdateBatchPacket>(batch);
                break;
            }

            case PacketType::DisconnectPacket: {
                std::cout << "Received disconnect packet" << '\n';
                DisconnectPacket dc_packet;
                des.object(dc_packet);
                std::cout << dc_packet.network_id.id << '\n';
                auto netid_entity = netid_to_entity.find(dc_packet.network_id);
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

    void disconnect() {
        enet_peer_disconnect(peer, 0);
        while (enet_host_service(client, &event, 0) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_DISCONNECT: {
                    return;
                }
                case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
                    return;
                }
                default: {
                    break;
                }
            }
        }
    }
};
