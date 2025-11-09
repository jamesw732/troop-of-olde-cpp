#pragma once
#include <cmath>

#include "flecs.h"
#include "raylib-cpp.hpp"

#include "components.hpp"

// TODO: Implement camera collision with terrain

inline void update_camera(flecs::world& world, raylib::Camera3D& camera) {
    auto local_player = world.lookup("LocalPlayer");
    if (!local_player.has<RenderPosition>()) {
        return;
    }
    float rot = local_player.get<RenderRotation>().val * PI / 180;
    // TODO: Variable height
    // TODO: Variable camera distance from player
    camera.position = local_player.get<RenderPosition>().val
        + raylib::Vector3{sin(rot), 1, cos(rot)} * 10;
    camera.target = local_player.get<RenderPosition>().val;
}
