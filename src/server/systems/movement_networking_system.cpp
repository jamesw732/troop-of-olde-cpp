#include <stdint.h>
#include <string.h>

#include "raylib-cpp.hpp"
#include "enet.h"

#include "movement_networking_system.hpp"

void movement_networking_system(ENetPeer* peer, Transformation t) {
    // Serialize position
    raylib::Vector3 pos = t.pos;
    uint8_t buf[sizeof(float) * 3];
    memcpy(buf, &pos.x, sizeof(float));
    memcpy(buf + sizeof(float), &pos.y, sizeof(float));
    memcpy(buf + 2 * sizeof(float), &pos.z, sizeof(float));
    ENetPacket* packet = enet_packet_create(buf, 3 * sizeof(float), 0);
    enet_peer_send(peer, 0, packet);
}
