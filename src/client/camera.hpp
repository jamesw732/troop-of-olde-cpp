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

inline void register_camera_input_system(flecs::world& world) {
    world.system<CamRotation, CamDistance, LocalPlayer>()
        .each([] (CamRotation& rot, CamDistance& dist, LocalPlayer) {
            process_camera_input(get_camera_input(), rot, dist);
        }
    );
}

// Updates the camera's position from character's rotation values
inline void update_camera(flecs::world& world, Camera3D& camera) {
    auto local_player = world.lookup("LocalPlayer");
    // Should CamRotation and CamDistance really be local_player components?
    // Should this be in a flecs query for LocalPlayer?
    if (!local_player.has<RenderPosition>()
        || !local_player.has<RenderRotation>()
        || !local_player.has<Scale>()
        || !local_player.has<CamRotation>()
        || !local_player.has<CamDistance>()) {
        return;
    }
    RenderPosition player_pos = local_player.get<RenderPosition>();
    RenderRotation player_rot = local_player.get<RenderRotation>();
    Scale player_scale = local_player.get<Scale>();
    CamRotation cam_rotation = local_player.get<CamRotation>();
    CamDistance cam_distance = local_player.get<CamDistance>();
    float y_rot = (player_rot.val.y + cam_rotation.y) * PI / 180;
    float polar_rot = (90 - cam_rotation.x) * PI / 180;
    Vector3 sphere_coords{
        sin(polar_rot) * sin(y_rot),
        cos(polar_rot),
        sin(polar_rot) * cos(y_rot)
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
