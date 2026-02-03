#pragma once
#include <iostream>
#include <cstdint>
#include <string>

#include "raylib.h"

#include "network_components.hpp"
#include "raylib-util.hpp"


struct Character {};

struct DisplayName {
    std::string name;
};

struct MovementInput {
    int8_t x = 0;
    int8_t z = 0;
    int8_t rot_y = 0;
    int16_t mouse_rot_y = 0;
    bool jump = false;
};

inline std::ostream& operator<<(std::ostream& os, const MovementInput& input) {
    os << "(x: " << (int) input.x << ", z: " << (int) input.z << ", rot_y: " << (int) input.rot_y << ")";
    return os;
}

struct SimPosition {
    Vector3 val{0};
};

struct SimRotation {
    Vector3 val{0};
};

struct Scale {
    Vector3 val{1, 1, 1};
};

struct Gravity {
    float val = 0;
};

struct Grounded {
    bool val = false;
};

struct ModelType {
    std::string name;
};

struct ModelPointer {
    Model* model;
};

struct ClientMoveTick {
    uint16_t val = 0;
};

struct PlayerSpawnState {
    uint32_t network_id;
    std::string name;
    Vector3 pos;
    Vector3 rot;
};

struct MovementUpdate {
    uint32_t network_id;
    uint16_t ack_tick;
    Vector3 pos;
    Vector3 rot;
    float gravity;
    bool grounded;
};

inline std::ostream& operator<<(std::ostream& os, const MovementUpdate& update) {
    os << "{ack_tick: " << update.ack_tick
        << ", pos: " << update.pos
        << ", rot: " << update.rot
        << ", gravity: " << update.gravity
        << ", grounded: " << update.grounded
        << "}";
    return os;
}

struct Terrain {};

