#pragma once
#include "shared/components/movement.hpp"
#include "shared/components/physics.hpp"


inline void process_movement_input(Position& pos, MovementInput input) {
    raylib::Vector3 velocity((float) input.x, 0, (float) input.z);
    velocity = velocity.Normalize();
    velocity = velocity * 0.25;

    pos.val += velocity;
}
