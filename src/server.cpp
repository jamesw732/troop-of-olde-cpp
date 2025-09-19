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
#include "shared/const.hpp"
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

    auto move_sys = world.system<Transformation, MovementInput>()
        .interval(MOVE_UPDATE_RATE)
        .each([](flecs::iter& it, size_t, Transformation& t, MovementInput& input) {
                movement_system(t, input);
            }
        );
    auto move_networking_sys = world.system<Connection, Transformation>()
        .interval(MOVE_UPDATE_RATE)
        .each([](flecs::iter& it, size_t, Connection& conn, Transformation& t) {
                movement_networking_system(conn.peer, t);
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
                    e.add<Transformation>();
                    e.set<Transformation>({
                         {0.0f, 0.0f, 0.0f},
                         {0.0f, 0.0f, 0.0f},
                         {1.0f, 1.0f, 1.0f}
                    });
                    e.add<MovementInput>();
                    e.set<MovementInput>({0, 0});
                    e.add<Connection>();
                    e.set<Connection>({event.peer});
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
                    auto id = (ecs_entity_t)(uintptr_t) event.peer->data;
                    flecs::entity e(world, id);
                    std::int8_t* data = reinterpret_cast<int8_t*>(event.packet->data);
                    MovementInput input;
                    input[0] = data[0];
                    input[1] = data[1];
                    e.set<MovementInput>(input);
                    std::cout << "Received movement input:"
                        << (int) input[0]
                        << ", "
                        << (int) input[1]
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