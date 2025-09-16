#include <cstdint>
#include <iostream>
#include <array>
#include <bitset>

#include "raylib-cpp.hpp"
#include "flecs.h"
#define ENET_IMPLEMENTATION
#include "enet.h"

#include "server/systems/movement_system.hpp"
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

    // TEMPORARY: Initialize player character
    // TODO: Character should be created by server
    Transformation transformComp = {
         {0.0f, 0.0f, 0.0f},
         {0.0f, 0.0f, 0.0f},
         {1.0f, 1.0f, 1.0f}
    };

    auto e = world.entity("character");
    e.add<Transformation>();
    e.set<Transformation>(transformComp);
    e.add<MovementInput>();
    e.set<MovementInput>({0, 0});

    // Main game loop
    while (true)
    {
        float dt = GetFrameTime();
        // Process networking events
        while (enet_host_service(server, &event, 0) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    std::cout << "A new client connected from "
                        <<  event.peer->address.host.__in6_u.__u6_addr8
                        << ":"
                        << event.peer->address.port
                        << "." << std::endl;
                    /* Store any relevant client information here. */
                    event.peer->data = (void*) "Client";
                    break;

                case ENET_EVENT_TYPE_RECEIVE: {
                    std::int8_t* data = reinterpret_cast<int8_t*>(event.packet->data);
                    MovementInput input;
                    input[0] = data[0];
                    input[1] = data[1];
                    e.set<MovementInput>(input);
                    // std::cout << "Received movement input:"
                    //     << (int) input[0]
                    //     << ", "
                    //     << (int) input[1]
                    //     << std::endl;
                    /* Clean up the packet now that we're done using it. */
                    enet_packet_destroy (event.packet);
                    break;
                }

                case ENET_EVENT_TYPE_DISCONNECT:
                    printf("%s disconnected.\n", (char*) event.peer->data);
                    /* Reset the peer's client information. */
                    event.peer->data = NULL;
                    break;

                case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                    printf("%s disconnected due to timeout.\n", (char*) event.peer->data);
                    /* Reset the peer's client information. */
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