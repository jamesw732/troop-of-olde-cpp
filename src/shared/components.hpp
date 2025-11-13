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
    int16_t mouse_rot_y = 0;
};

inline std::ostream& operator<<(std::ostream& os, const MovementInput& input) {
    os << "(x: " << (int) input.x << ", z: " << (int) input.z << ", rot_y: " << (int) input.rot_y << ")";
    return os;
}

struct SimPosition {
    raylib::Vector3 val;
};

struct SimRotation {
    float val = 0;
};

struct Scale {
    raylib::Vector3 val{1, 1, 1};
};

struct ModelName {
    std::string name;
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

struct Terrain {};
