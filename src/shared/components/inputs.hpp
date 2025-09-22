#pragma once
#include <array>
#include <cstdint>
#include <vector>


struct MovementInput {
    int8_t x;
    int8_t z;
};

struct InputPacket {
    uint16_t tick;
    std::vector<MovementInput> inputs;
};
