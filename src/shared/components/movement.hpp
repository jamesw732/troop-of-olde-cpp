#pragma once
#include <cstdint>


/*
 * Represents the state of WASD for a single movement tick
 */
struct MovementInput {
    int8_t x = 0;
    int8_t z = 0;
};
