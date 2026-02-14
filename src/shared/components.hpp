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
    os << "MovementInput{\n";
    os << indent();
    print_indent(os);
    os << "x: " << (int) input.x << "\n";
    print_indent(os);
    os << "z: " << (int) input.z << "\n";
    print_indent(os);
    os << "rot_y: " << (int) input.rot_y << "\n";
    print_indent(os);
    os << "mouse_rot_y: " << (int) input.mouse_rot_y << "\n";
    print_indent(os);
    os << "jump: " <<  input.jump << "\n";
    os << unindent();
    print_indent(os);
    os << "}";
    return os;
}

inline bool operator==(MovementInput input1, MovementInput input2) {
    return input1.x == input2.x && input1.z == input2.z
        && input1.jump == input2.jump && input1.mouse_rot_y == input2.mouse_rot_y
        && input1.rot_y == input2.mouse_rot_y;
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

inline std::ostream& operator<<(std::ostream& os, const PlayerSpawnState& state) {
    os << "PlayerSpawnState{\n";
    os << indent();
    print_indent(os);
    os << "network_id: " << state.network_id << "\n";
    print_indent(os);
    os << "name: " << state.name << "\n";
    print_indent(os);
    os << "pos: " << state.pos << "\n";
    print_indent(os);
    os << "rot: " << state.rot << "\n";
    os << unindent();
    print_indent(os);
    os << "}";

    return os;
}

struct MovementUpdate {
    uint32_t network_id;
    uint16_t ack_tick;
    Vector3 pos;
    Vector3 rot;
    float gravity;
    bool grounded;
};

inline std::ostream& operator<<(std::ostream& os, const MovementUpdate& update) {
    os << "MovementUpdate{\n";
    os << indent();
    print_indent(os);
    os << "network_id: " << update.network_id << "\n";
    print_indent(os);
    os << "ack_tick: " << update.ack_tick << "\n";
    print_indent(os);
    os << "pos: " << update.pos << "\n";
    print_indent(os);
    os << "rot: " << update.rot << "\n";
    print_indent(os);
    os << "gravity: " << update.gravity << "\n";
    print_indent(os);
    os << "grounded: " << update.grounded << "\n";
    os << unindent();
    print_indent(os);
    os << "}";
    return os;
}

struct Terrain {};

