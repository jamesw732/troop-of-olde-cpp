#include "enet.h"

#include "movement_networking_system.hpp"
#include "shared/components/physics.hpp"


void movement_networking_system(ENetPeer* peer, MovementInput& input) {
    ENetPacket* packet = enet_packet_create(input.data(), input.size() * sizeof(std::int8_t), 0);
    enet_peer_send(peer, 0, packet);
}
