#pragma once
#include <cmath>

#include "raylib.h"
#include "raymath.h"

#include "animation.hpp"
#include "const.hpp"
#include "physics.hpp"
#include "components.hpp"
#include "util.hpp"


inline Vector3 get_input_displacement(const MovementInput& input, float rot) {
    Vector3 disp{(float) -input.x, 0, (float) -input.z};
    disp = Vector3Normalize(disp);
    disp = Vector3RotateByAxisAngle(disp, {0, 1, 0}, rot * PI / 180);
    disp = Vector3Scale(disp, 0.25);

    return disp;
}

inline void apply_input_rotation(const MovementInput& input, float& rot) {
    rot += input.rot_y * 5;
    rot -= input.mouse_rot_y;
    rot = fmodf(rot, 360.0);
}

inline void apply_input_jump(const MovementInput& input, float& gravity, bool& grounded) {
    if (input.jump && grounded) {
        gravity = 0.3;
        grounded = false;
    }
}

/*
 * Entry point for movement processing
 * Each call to this increments a single character's movement by one tick
 */
inline void tick_movement(
    flecs::world& world,
    const MovementInput& input,
    Vector3& pos,
    float& rot,
    float& gravity,
    bool& grounded
) {
    update_gravity(gravity, grounded);
    Vector3 disp = get_input_displacement(input, rot);
    apply_input_rotation(input, rot);
    // TODO: Let the player jump if they recently left the ground
    apply_input_jump(input, gravity, grounded);
    disp.y += gravity;
    Vector3 collision_disp = process_collision(world, pos, disp, Vector3Length(disp));
    check_beneath(world, pos, grounded);
    pos = Vector3Add(pos, collision_disp);
    if (!Vector3Equals(disp, collision_disp)) {
        grounded = true;
    }
}
