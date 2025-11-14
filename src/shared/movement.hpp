#pragma once
#include <cmath>

#include "raylib-cpp.hpp"

#include "components.hpp"
#include "const.hpp"
#include "util.hpp"


inline raylib::Vector3 process_movement_input(MovementInput input, float& rot) {
    raylib::Vector3 disp{(float) input.x, 0, (float) input.z};
    disp = disp.Normalize();
    disp = Vector3RotateByAxisAngle(disp, {0, 1, 0}, rot * PI / 180);
    disp = disp * 0.25;

    rot += input.rot_y * 5;
    rot -= input.mouse_rot_y;
    rot = fmodf(rot, 360.0);

    return disp;
}
