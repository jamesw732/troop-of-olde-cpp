#pragma once
#include "raylib-cpp.hpp"

#include "shared/components/physics.hpp"
#include "shared/components/inputs.hpp"

inline void movement_system(Position& pos, MovementInput& input) {
    raylib::Vector3 velocity((float) input[0], 0, (float) input[1]);
    velocity = velocity.Normalize();
    velocity = velocity * 0.25;

    pos += velocity;
}
