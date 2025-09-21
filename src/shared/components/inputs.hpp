#pragma once
#include <array>
#include <cstdint>
#include <vector>


typedef std::array<std::int8_t, 2> MovementInput;

struct InputPacket {
    uint16_t tick;
    std::vector<MovementInput> inputs;
};
