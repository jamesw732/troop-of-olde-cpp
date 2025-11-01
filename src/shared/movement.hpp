#pragma once
#include <cmath>

#include "raylib-cpp.hpp"

#include "components.hpp"
#include "util.hpp"


inline void process_movement_input(raylib::Vector3& pos, float& rot, MovementInput input) {
    raylib::Vector3 velocity((float) input.x, 0, (float) input.z);
    velocity = velocity.Normalize();
    velocity = Vector3RotateByAxisAngle(velocity, {0, 1, 0}, rot * PI / 180);
    velocity = velocity * 0.25;

    pos += velocity;

    rot += input.rot_y * 5;
    rot = fmodf(rot, 360.0);
}
