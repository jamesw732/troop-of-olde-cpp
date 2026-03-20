#pragma once
#include <cmath>

#include "flecs.h"
#include "raylib.h"
#include "raymath.h"

#include "../shared/const.hpp"
#include "../shared/physics.hpp"
#include "components.hpp"
#include "input.hpp"


inline void process_camera_input(const CameraInput& input, CamRotation& rot, CamDistance& dist) {
    float new_rot_x = rot.x + input.rot_x;
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

inline void register_camera_input_system(flecs::world& world) {
    world.system<CamRotation, CamDistance, LocalPlayer>()
        .each([] (CamRotation& rot, CamDistance& dist, LocalPlayer) {
            process_camera_input(get_camera_input(), rot, dist);
        }
    );
}

// Updates the camera's position from character's position and rotation values
inline void register_camera_update_system(flecs::world& world, Camera3D& camera) {
    world.system<LocalPlayer, RenderPosition, RenderRotation, Scale,
                 CamRotation, CamDistance>()
         .each([&] (LocalPlayer, RenderPosition player_pos, RenderRotation player_rot, Scale player_scale,
                    CamRotation cam_rotation, CamDistance cam_distance) {
            float y_rot = (player_rot.val.y + cam_rotation.y) * PI / 180;
            float x_rot = (90 - cam_rotation.x) * PI / 180;
            Vector3 sphere_coords{
                -sin(x_rot) * sin(y_rot),
                cos(x_rot),
                -sin(x_rot) * cos(y_rot)
            };
            Vector3 focus_pos = Vector3Add(player_pos.val, {0, player_scale.val.y * 0.5f, 0});
            camera.position = Vector3Add(focus_pos, Vector3Scale(sphere_coords, cam_distance.val));
            RayCollision camera_collision = find_closest_collision(
                world, focus_pos,
                Vector3Subtract(camera.position, focus_pos)
            );
            if (camera_collision.hit && camera_collision.distance < cam_distance.val) {
                camera.position = camera_collision.point;
            }
            camera.target = focus_pos;
        }
    );
}
