#include <iostream>

#include "enet.h"

#include "client/systems/movement_systems.hpp"
#include "shared/components/movement.hpp"
#include "shared/util.hpp"


class Network {
  public:
    ENetHost* client = {0};
    ENetPeer* peer = {0};
    ENetAddress address = {0};
    ENetEvent event;
    flecs::world world;

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
        if (enet_host_service(client, &event, 5000) > 0 &&
            event.type == ENET_EVENT_TYPE_CONNECT) {
            std::cout << "Connection to Troop of Olde server succeeded." << std::endl;
        } else {
            enet_peer_reset(peer);
            std::cout << "Connection to Troop of Olde server failed." << std::endl;
        }
    }

    void process_events() {
        while (enet_host_service(client, &event, 0) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    break;
                }
                case ENET_EVENT_TYPE_RECEIVE: {
                    process_recv_event();
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

    void process_recv_event() {
        flecs::entity player_e = world.lookup("LocalPlayer");
        ServerMovementUpdate& move_update = player_e.get_mut<ServerMovementUpdate>();
        Buffer buffer;
        auto state = bitsery::quickDeserialization(
            InputAdapter{
                event.packet->data,
                event.packet->dataLength
            },
            move_update
        );
        std::cout << "Received position "
            << vector3_to_string(move_update.pos)
            << " from server for tick "
            << (int) move_update.ack_tick
            << std::endl;
    }
};
