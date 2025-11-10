#pragma once
#include <cmath>

#include "flecs.h"
#include "raylib-cpp.hpp"

#include "../shared/const.hpp"
#include "components.hpp"
#include "input.hpp"

// TODO: Implement camera collision with terrain

inline void process_camera_input(const int8_t& input, float& x_rot) {
    float new_rot = x_rot + 2 * input;
    new_rot = Clamp(new_rot, -88, 88);
    x_rot = new_rot;
}

// Updates the camera's position from character's rotation values
inline void update_camera(flecs::world& world, raylib::Camera3D& camera) {
    auto local_player = world.lookup("LocalPlayer");
    if (!local_player.has<RenderPosition>() || !local_player.has<HeadXRotation>()) {
        return;
    }
    float y_rot = local_player.get<RenderRotation>().val * PI / 180;
    float polar_rot = (90 - local_player.get<HeadXRotation>().val) * PI / 180;
    raylib::Vector3 sphere_coords{
        sin(polar_rot) * sin(y_rot),
        cos(polar_rot),
        sin(polar_rot) * cos(y_rot)
    };
    // TODO: Variable height
    // TODO: Variable camera distance from player
    camera.position = local_player.get<RenderPosition>().val
        + sphere_coords * 15;
        // + raylib::Vector3{sin(rot), 1, cos(rot)} * 10;
    camera.target = local_player.get<RenderPosition>().val;
}
