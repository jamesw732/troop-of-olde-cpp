#pragma once
#include <cmath>

#include "raylib-cpp.hpp"

#include "components.hpp"
#include "const.hpp"
#include "physics.hpp"
#include "util.hpp"


inline raylib::Vector3 process_movement_input(
    const MovementInput& input,
    float& rot,
    float& gravity,
    bool& grounded)
{
    raylib::Vector3 disp{(float) input.x, 0, (float) input.z};
    disp = disp.Normalize();
    disp = Vector3RotateByAxisAngle(disp, {0, 1, 0}, rot * PI / 180);
    disp = disp * 0.25;

    rot += input.rot_y * 5;
    rot -= input.mouse_rot_y;
    rot = fmodf(rot, 360.0);

    if (input.jump && grounded) {
        gravity = 0.3;
        grounded = false;
    }

    return disp;
}

inline void tick_movement(
    flecs::world& world,
    raylib::Vector3& pos,
    float& rot,
    const MovementInput& input,
    float& gravity,
    bool& grounded)
{
    update_gravity(gravity, grounded);
    raylib::Vector3 disp = process_movement_input(input, rot, gravity, grounded);
    disp.y += gravity;
    process_collision(world, pos, disp, grounded);
    pos += disp;
}
