#pragma once
#include <cstdint>
#include <string>

#include "Vector3.hpp"
#include "network_components.hpp"

struct DisplayName {
    std::string name;
};

struct MovementInput {
    int8_t x = 0;
    int8_t z = 0;
};

struct Position {
    raylib::Vector3 val{0, 0, 0};
};

struct ClientMoveTick {
    uint16_t val = 0;
};

struct PlayerSpawnState {
    NetworkId network_id;
    DisplayName name;
    Position pos;
};

struct MovementUpdate {
    NetworkId network_id;
    ClientMoveTick ack_tick;
    Position pos;
};
