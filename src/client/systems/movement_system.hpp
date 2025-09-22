#pragma once
#include "raylib-cpp.hpp"
#include "enet.h"

#include "shared/components/physics.hpp"
#include "shared/components/inputs.hpp"



inline void movement_system(Position& pos, MovementInput& input) {
    raylib::Vector3 velocity((float) input.x, 0, (float) input.z);
    velocity = velocity.Normalize();
    velocity = velocity * 0.25;

    pos += velocity;
}
