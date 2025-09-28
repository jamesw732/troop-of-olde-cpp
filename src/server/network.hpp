#pragma once
#include <iostream>

#include "enet.h"
#include "flecs.h"
#include "server/components/networking.hpp"
#include "shared/components/movement.hpp"
#include "shared/components/physics.hpp"
#include "shared/components/ticks.hpp"
#include "shared/serialize/helpers.hpp"

class Network {
  public:
    ENetHost* server = {0};
    ENetPeer* peer = {0};
    ENetAddress address = {0};
    ENetEvent event;
    flecs::world world;

    Network(flecs::world& w) : world(w) {};

    bool create() {
        // Create ENet server
        if (enet_initialize() != 0) {
            std::cout << "An error occurred while initializing ENET." << std::endl;
            return 1;
        }
        address.host = ENET_HOST_ANY;
        address.port = 7777;
        #define MAX_CLIENTS 32
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
                        << event.peer->address.port
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
                        << (ecs_entity_t)(uintptr_t) event.peer->data
                        << " disconnected."
                        << std::endl;
                    event.peer->data = NULL;
                    break;

                case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                    std::cout << "Client with Entity id "
                        << (ecs_entity_t)(uintptr_t) event.peer->data
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
        e.set<Position>({{0.0f, 0.0f, 0.0f,}});
        e.add<MovementInputPacket>();
        e.set<MovementInputPacket>({0, {}});
        e.add<Connection>();
        e.set<Connection>({event.peer});
        e.add<ClientMoveTick>();
        e.set<ClientMoveTick>({0});
        event.peer->data = (void*) e.raw_id();
    }

    void process_recv_event() {
        MovementInputPacket input_packet;
        Buffer buffer;
        auto state = bitsery::quickDeserialization(
            InputAdapter{
                event.packet->data,
                event.packet->dataLength
            },
            input_packet
        );
        auto id = (ecs_entity_t)(uintptr_t) event.peer->data;
        flecs::entity e(world, id);
        MovementInput input = input_packet.inputs.back();
        e.set<MovementInputPacket>(input_packet);
        uint16_t tick = input_packet.tick;
        std::cout << "Received "
            << input_packet.inputs.size()
            << " movement inputs up to tick "
            << tick
            << ". Most recent: "
            << (int) input.x
            << ", "
            << (int) input.z
            << std::endl;
        /* Clean up the packet now that we're done using it. */
        enet_packet_destroy(event.packet);
    }
};
