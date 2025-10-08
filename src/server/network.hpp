#pragma once
#include <iostream>

#include "enet.h"
#include "flecs.h"
#include "server/components.hpp"
#include "shared/components.hpp"
#include "shared/packets.hpp"
#include "shared/serialize.hpp"
#include "shared/const.hpp"

class Network {
  public:
    ENetHost* server = {0};
    ENetPeer* peer = {0};
    ENetAddress address = {0};
    ENetEvent event;
    flecs::world world;
    uint32_t network_id_counter = 0;

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
                    create_character();
                    break;
                }

                case ENET_EVENT_TYPE_RECEIVE: {
                    process_recv_event();
                    break;
                }

                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << "Client with Entity id "
                        << cast_raw_id(event.peer->data)
                        << " disconnected."
                        << std::endl;
                    event.peer->data = NULL;
                    break;

                case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                    std::cout << "Client with Entity id "
                        << cast_raw_id(event.peer->data)
                        << " disconnected due to timeout."
                        << std::endl;
                    event.peer->data = NULL;
                    break;

                case ENET_EVENT_TYPE_NONE:
                    break;
            }
        }
    }
    
    void create_character() {
        auto e = world.entity();
        e.add<Position>();
        e.add<MovementInputPacket>();
        e.set<Connection>({event.peer});
        e.add<ClientMoveTick>();
        e.add<DisplayName>();
        e.set<NetworkId>({network_id_counter});
        network_id_counter++;
        event.peer->data = (void*) e.raw_id();
    }

    void process_recv_event() {
        auto id = cast_raw_id(event.peer->data);
        flecs::entity e(world, id);
        handle_packet(e, event.packet->data, event.packet->dataLength);
        /* Clean up the packet now that we're done using it. */
        enet_packet_destroy(event.packet);
    }

    ecs_entity_t cast_raw_id(void* raw_id) {
        return (ecs_entity_t)(uintptr_t) raw_id;
    }
};
