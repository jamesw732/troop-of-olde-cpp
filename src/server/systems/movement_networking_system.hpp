#pragma once
#include "enet.h"

#include "shared/components/physics.hpp"
#include "shared/components/ticks.hpp"
#include "shared/components/movement.hpp"
#include "shared/serialize/serialize_vector3.hpp"


inline void movement_networking_system(ENetPeer* peer, Position& pos, ClientMoveTick& tick) {
    ServerMovementUpdate move_update{tick.val, pos.val};
    // Serialize position
    Buffer buffer;
    size_t size = bitsery::quickSerialization(OutputAdapter{buffer}, move_update);
    // Create packet and send to client
    ENetPacket* packet = enet_packet_create(buffer.data(), size, 0);
    enet_peer_send(peer, 0, packet);
}
