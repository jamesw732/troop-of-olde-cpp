#include <cstdint>
#include <iostream>
#include <array>
#include <bitset>
#include <string>

#include "raylib-cpp.hpp"
#include "flecs.h"
#define ENET_IMPLEMENTATION
#include "enet.h"

#include "server/components/networking.hpp"
#include "server/systems/movement_system.hpp"
#include "server/systems/movement_networking_system.hpp"
#include "shared/components/physics.hpp"
#include "shared/components/inputs.hpp"
#include "shared/components/ticks.hpp"
#include "shared/const.hpp"
#include "shared/serialize/serialize_input_packet.hpp"
#include "shared/util.hpp"


int main(void)
{
    // Create ENet server
    if (enet_initialize() != 0) {
        std::cout << "An error occurred while initializing ENET." << std::endl;
        return 1;
    }
    ENetAddress address = {0};

    address.host = ENET_HOST_ANY;
    address.port = 7777;

    #define MAX_CLIENTS 32
    ENetHost* server = enet_host_create(&address, MAX_CLIENTS, 2, 0, 0);

    if (server == NULL) {
        std::cout << "An error occurred while trying to create an ENet server host." << std::endl;
        return 1;
    }
    std::cout << "Started a server..." << std::endl;
    ENetEvent event;
    // Initialize ECS world and systems
    flecs::world world;

    auto move_sys = world.system<Position, ClientMoveTick, InputPacket>()
        .interval(MOVE_UPDATE_RATE)
        .each([](Position& pos, ClientMoveTick& tick, InputPacket& packet) {
                movement_system(pos, tick, packet);
            }
        );
    auto move_networking_sys = world.system<Connection, Position>()
        .interval(MOVE_UPDATE_RATE)
        .each([](flecs::iter& it, size_t, Connection& conn, Position& pos) {
                movement_networking_system(conn.peer, pos);
            }
        );

    // Main game loop
    while (true)
    {
        float dt = GetFrameTime();
        // Process networking events
        while (enet_host_service(server, &event, 0) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    std::cout << "A new client connected from "
                        <<  event.peer->address.host.__in6_u.__u6_addr8
                        << ":"
                        << event.peer->address.port
                        << "." << std::endl;
                    auto e = world.entity();
                    e.add<Position>();
                    e.set<Position>({0.0f, 0.0f, 0.0f,});
                    e.add<InputPacket>();
                    e.set<InputPacket>({0, {}});
                    e.add<Connection>();
                    e.set<Connection>({event.peer});
                    e.add<ClientMoveTick>();
                    e.set<ClientMoveTick>(0);
                    event.peer->data = (void*) e.raw_id();
                    break;
                }

                case ENET_EVENT_TYPE_RECEIVE: {
                  /* 
                   * Handle movement input from client, doesn't handle any other packets
                   * Assumes movement inputs are evenly spaced, once per movement tick,
                   * and always arrive in order
                   * Eventually, these will be fed into a movement input buffer
                   */
                    InputPacket input_packet;
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
                    e.set<InputPacket>(input_packet);
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
                    enet_packet_destroy (event.packet);
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
        world.progress(dt);
        // Transformation trans = e.get<Transformation>();
        // raylib::Vector3 pos = trans.pos;
        // std::cout << vector3_to_string(pos) << std::endl;
    }

    return 0;
}