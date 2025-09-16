#include "raylib-cpp.hpp"
#include "flecs.h"
#include "enet.h"

#include "movement_system.hpp"
#include "shared/components/physics.hpp"

void movement_system(ENetPeer* peer, Transformation& t, MovementInput& input) {
    raylib::Vector3 velocity((float) input[0], 0, (float) input[1]);
    velocity = velocity.Normalize();
    velocity = velocity * 0.25;

    t.pos += velocity;

    ENetPacket* packet = enet_packet_create(input.data(), input.size() * sizeof(std::int8_t), 0);
    enet_peer_send(peer, 0, packet);
}

