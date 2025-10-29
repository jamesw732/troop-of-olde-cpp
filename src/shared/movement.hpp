#pragma once
#include <cmath>

#include "raylib-cpp.hpp"

#include "components.hpp"
#include "raymath.h"


inline void process_movement_input(raylib::Vector3& pos, MovementInput input) {
    raylib::Vector3 velocity((float) input.x, 0, (float) input.z);
    velocity = velocity.Normalize();
    velocity = velocity * 0.25;

    pos += velocity;
}

inline void process_movement_input(raylib::Vector3& pos, float& rot, MovementInput input) {
    process_movement_input(pos, input);
    rot += input.rot_y * 5;
    rot = fmodf(rot, 360.0);
}
