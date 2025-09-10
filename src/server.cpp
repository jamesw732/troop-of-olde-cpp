#include "raylib.h"
#include "flecs.h"
#define ENET_IMPLEMENTATION
#include "enet.h"
#include <iostream>

const float MOVE_UPDATE_RATE = 1.0 / 20.0;

struct Transformation {
    Vector3 pos;
    Vector3 rot;
    Vector3 scale;
};

int main(void)
{
    // Initialize ECS world and systems
    flecs::world world;

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

                case ENET_EVENT_TYPE_RECEIVE:
                    std::cout << "A packet of length "
                        << event.packet->dataLength
                        << " containing "
                        << event.packet->data
                        << " was received from "
                        << (char*) event.peer->data
                        << " on channel "
                        << event.channelID
                        << std::endl;
                    /* Clean up the packet now that we're done using it. */
                    enet_packet_destroy (event.packet);
                    break;

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
    }

    return 0;
}