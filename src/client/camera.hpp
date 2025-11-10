#pragma once
#include <cmath>

#include "flecs.h"
#include "raylib-cpp.hpp"

#include "../shared/const.hpp"
#include "components.hpp"
#include "input.hpp"

// TODO: Implement camera collision with terrain

inline void process_camera_input(const int8_t& input, float& x_rot) {
    float new_rot = x_rot + 100 * input * GetFrameTime();
    new_rot = Clamp(new_rot, -88, 88);
    x_rot = new_rot;
}

inline void register_camera_input_system(flecs::world& world, InputHandler& input_handler) {
    world.system<CamRotation, LocalPlayer>()
        .each([&input_handler] (CamRotation& rot, LocalPlayer) {
            process_camera_input(input_handler.get_updown_keyboard_rotation(), rot.x);
        }
    );
}

// Updates the camera's position from character's rotation values
inline void update_camera(flecs::world& world, raylib::Camera3D& camera) {
    auto local_player = world.lookup("LocalPlayer");
    if (!local_player.has<RenderPosition>() || !local_player.has<CamRotation>()) {
        return;
    }
    float y_rot = local_player.get<RenderRotation>().val * PI / 180;
    float polar_rot = (90 - local_player.get<CamRotation>().x) * PI / 180;
    raylib::Vector3 sphere_coords{
        sin(polar_rot) * sin(y_rot),
        cos(polar_rot),
        sin(polar_rot) * cos(y_rot)
    };
    // TODO: Variable camera distance from player
    camera.position = local_player.get<RenderPosition>().val
        + sphere_coords * 15;
    camera.target = local_player.get<RenderPosition>().val;
}
