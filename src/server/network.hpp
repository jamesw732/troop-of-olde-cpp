#pragma once
#include <deque>
#include <iostream>
#include <unordered_map>

#define ENET_IMPLEMENTATION
#include "enet.h"
#include "flecs.h"

#include "shared/network_components.hpp"
#include "shared/const.hpp"
#include "shared/util.hpp"

struct RecvPacket {
    flecs::entity e;
    std::vector<uint8_t> packet_data;
};

class Network {
  private:
    uint32_t network_id_counter = 0;
    flecs::world world;
    ENetHost* server = {0};
    ENetAddress address = {0};
    ENetEvent event;
    std::unordered_map<NetworkId, ENetPeer*> netid_to_peer;

  public:
    std::deque<RecvPacket> packets;

    Network(flecs::world& w) : world(w) {};

    bool create() {
        // Create ENet server
        if (enet_initialize() != 0) {
            std::cout << "An error occurred while initializing ENET." << std::endl;
            return 1;
        }
        address.host = ENET_HOST_ANY;
        address.port = 7777;
        server = enet_host_create(&address, MAX_CLIENTS, 2, 0, 0);

        if (server == NULL) {
            std::cout << "An error occurred while trying to create an ENet server host." << std::endl;
            return 1;
        }
        std::cout << "Started a server..." << std::endl;
        ENetEvent event;
        return 0;
    }

    void process_events() {
        // Process networking events
        while (enet_host_service(server, &event, 0) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    std::cout << "A new client connected from "
                        <<  event.peer->address.host.__in6_u.__u6_addr8
                        << ":"
                        << (int) event.peer->address.port
                        << "." << std::endl;
                    auto entity = world.entity();
                    entity.set<NetworkId>({network_id_counter});
                    netid_to_peer[{network_id_counter}] = event.peer;
                    network_id_counter++;
                    event.peer->data = (void*) entity.raw_id();
                    break;
                }

                case ENET_EVENT_TYPE_RECEIVE: {
                    // handle_packet();
                    auto id = cast_raw_id(event.peer->data);
                    flecs::entity entity(world, id);
                    uint8_t* buffer = event.packet->data;
                    size_t size = event.packet->dataLength;
                    std::vector<uint8_t> packet_data(buffer, buffer + size);
                    RecvPacket recv_packet{entity, packet_data};
                    packets.push_back(recv_packet);
                    break;
                }

                case ENET_EVENT_TYPE_DISCONNECT: {
                    std::cout << "Client with Entity id "
                        << cast_raw_id(event.peer->data)
                        << " disconnected."
                        << std::endl;
                    auto id = cast_raw_id(event.peer->data);
                    flecs::entity entity(world, id);
                    entity.add<Disconnected>();
                    event.peer->data = NULL;
                    enet_peer_reset(event.peer);
                    break;
                }

                case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
                    std::cout << "Client with Entity id "
                        << cast_raw_id(event.peer->data)
                        << " disconnected due to timeout."
                        << std::endl;
                    auto id = cast_raw_id(event.peer->data);
                    flecs::entity entity(world, id);
                    entity.add<Disconnected>();
                    event.peer->data = NULL;
                    enet_peer_reset(event.peer);
                    break;
                }

                case ENET_EVENT_TYPE_NONE:
                    break;
            }
        }
    }

    void queue_data_unreliable(const NetworkId& network_id, const Buffer& buffer, const size_t size){
        auto netid_peer = netid_to_peer.find(network_id);
        if (netid_peer == netid_to_peer.end()) {
            dbg("Failed to find peer");
            return;
        }
        ENetPeer* peer = netid_peer->second;
        ENetPacket* packet = enet_packet_create(buffer.data(), size, 0);
        enet_peer_send(peer, 0, packet);
    }

    void queue_data_reliable(const NetworkId& network_id, const Buffer& buffer, const size_t size){
        auto netid_peer = netid_to_peer.find(network_id);
        if (netid_peer == netid_to_peer.end()) {
            dbg("Failed to find peer");
            return;
        }
        ENetPeer* peer = netid_peer->second;
        ENetPacket* packet = enet_packet_create(buffer.data(), size, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 1, packet);
    }

    void send_network_buffer() {
        enet_host_flush(server);
    }

    ecs_entity_t cast_raw_id(void* raw_id) {
        return (ecs_entity_t)(uintptr_t) raw_id;
    }
};
