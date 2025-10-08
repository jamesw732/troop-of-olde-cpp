#pragma once
#include <cstdint>
#include <string>

#include "raylib-cpp.hpp"

struct DisplayName {
    std::string name = "";
};

struct NetworkId {
    uint32_t id;
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
    Position pos{{0, 0, 0}};
};

