#pragma once
#include "raylib-cpp.hpp"

#include "shared/components.hpp"


inline void process_movement_input(raylib::Vector3& pos, MovementInput input) {
    raylib::Vector3 velocity((float) input.x, 0, (float) input.z);
    velocity = velocity.Normalize();
    velocity = velocity * 0.25;

    pos += velocity;
}
