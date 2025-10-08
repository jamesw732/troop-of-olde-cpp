#pragma once
#include <iostream>

#include "enet.h"
#include "flecs.h"
#include "server/character.hpp"
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
                    break;
                }

                case ENET_EVENT_TYPE_RECEIVE: {
                    handle_packet();
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

    void handle_packet() {
        uint8_t* buffer = event.packet->data;
        size_t size = event.packet->dataLength;
        bitsery::Deserializer<InputAdapter> des{InputAdapter{buffer, size}};
        PacketType pkt_type;
        des.value1b(pkt_type);
        switch (pkt_type) {
            case PacketType::MovementInput: {
                auto id = cast_raw_id(event.peer->data);
                flecs::entity entity(world, id);
                MovementInputPacket input_packet;
                des.object(input_packet);
                entity.set<MovementInputPacket>(input_packet);

                // MovementInput input = input_packet.inputs.back();
                // uint16_t tick = input_packet.tick;
                // std::cout << "Received "
                //     << input_packet.inputs.size()
                //     << " movement inputs up to tick "
                //     << tick
                //     << ". Most recent: "
                //     << (int) input.x
                //     << ", "
                //     << (int) input.z
                //     << std::endl;
                break;
            }

            case PacketType::ClientLoginPacket: {
                // std::cout << "Received login packet" << std::endl;
                ClientLoginPacket login;
                des.object(login);
                auto e = ::create_character(world);
                e.set<DisplayName>(login.name);
                e.set<Connection>({event.peer});
                e.set<NetworkId>({network_id_counter});
                network_id_counter++;
                event.peer->data = (void*) e.raw_id();
                e.add<NeedsSpawnBatch>();
                break;
            }

            default: {
                break;
            }
        }

    }

    ecs_entity_t cast_raw_id(void* raw_id) {
        return (ecs_entity_t)(uintptr_t) raw_id;
    }
};
