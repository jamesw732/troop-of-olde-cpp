#pragma once
#include <cmath>

#include "flecs.h"
#include "raylib-cpp.hpp"

#include "../shared/const.hpp"
#include "components.hpp"
#include "input.hpp"

// TODO: Implement camera collision with terrain


inline void process_camera_input(const CameraInput& input, CamRotation& rot, CamDistance& dist) {
    float new_rot_x = rot.x + 100 * input.rot_x * GetFrameTime();
    new_rot_x += input.mouse_rot.x;
    new_rot_x = Clamp(new_rot_x, -88, 88);
    rot.x = new_rot_x;
    if (input.reset) {
        rot.y = 0;
    }
    else {
        rot.y -= input.mouse_rot.y;
    }
    dist.val = Clamp(dist.val + input.scroll, 0, 20);
}

inline void register_camera_input_system(flecs::world& world, InputHandler& input_handler) {
    world.system<CamRotation, CamDistance, LocalPlayer>()
        .each([&input_handler] (CamRotation& rot, CamDistance& dist, LocalPlayer) {
            process_camera_input(input_handler.get_camera_input(), rot, dist);
        }
    );
}

// Updates the camera's position from character's rotation values
inline void update_camera(flecs::world& world, raylib::Camera3D& camera) {
    auto local_player = world.lookup("LocalPlayer");
    if (!local_player.has<RenderPosition>() || !local_player.has<CamRotation>()) {
        return;
    }
    CamRotation cam_rotation = local_player.get<CamRotation>();
    CamDistance cam_distance = local_player.get<CamDistance>();
    float y_rot = (local_player.get<RenderRotation>().val + cam_rotation.y) * PI / 180;
    float polar_rot = (90 - cam_rotation.x) * PI / 180;
    raylib::Vector3 sphere_coords{
        sin(polar_rot) * sin(y_rot),
        cos(polar_rot),
        sin(polar_rot) * cos(y_rot)
    };
    // TODO: Variable camera distance from player
    camera.position = local_player.get<RenderPosition>().val
        + sphere_coords * cam_distance.val;
    camera.target = local_player.get<RenderPosition>().val;
}
