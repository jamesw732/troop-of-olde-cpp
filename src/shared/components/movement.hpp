#pragma once
#include <array>
#include <cstdint>
#include <vector>

#include "raylib-cpp.hpp"

/*
 * Represents the state of WASD for a single movement tick
 */
struct MovementInput {
    int8_t x;
    int8_t z;
};

/*
 * Contains client's  movement tick and all unacknowledged MovementInputs
 * This is serialized and sent to the server.
 */
struct MovementInputPacket {
    // The current movement tick, last movement tick in the buffer
    uint16_t tick;
    // All movement inputs in the buffer
    std::vector<MovementInput> inputs;
};

/*
 * Contains the server's currently acknowledged tick and the corresponding position
 */
struct ServerMovementUpdate {
    uint16_t ack_tick;
    raylib::Vector3 pos;
};
