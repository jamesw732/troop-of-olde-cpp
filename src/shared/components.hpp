#pragma once
#include <iostream>
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
    int8_t rot_y = 0;
};

struct SimPosition {
    raylib::Vector3 val;
};

inline std::ostream& operator<<(std::ostream& os, const SimPosition& pos) {
    os << "(" << pos.val.x << ", " << pos.val.y << ", " << pos.val.z << ")";
    return os;
}

struct SimRotation {
    float val = 0;
};

struct ClientMoveTick {
    uint16_t val = 0;
};

struct PlayerSpawnState {
    NetworkId network_id;
    DisplayName name;
    SimPosition pos;
    SimRotation rot;
};

struct MovementUpdate {
    NetworkId network_id;
    ClientMoveTick ack_tick;
    SimPosition pos;
    SimRotation rot;
};
