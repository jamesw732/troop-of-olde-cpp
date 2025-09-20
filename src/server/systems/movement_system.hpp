#pragma once
#include "raylib-cpp.hpp"

#include "shared/components/physics.hpp"

inline void movement_system(Transformation& t, MovementInput& input) {
    raylib::Vector3 velocity((float) input[0], 0, (float) input[1]);
    velocity = velocity.Normalize();
    velocity = velocity * 0.25;

    t.pos += velocity;
}
