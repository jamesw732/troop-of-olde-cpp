#pragma once
#include <cmath>

#include "flecs.h"
#include "raylib-cpp.hpp"

#include "components.hpp"

inline void update_camera(flecs::world& world, raylib::Camera3D& camera) {
    auto local_player = world.lookup("LocalPlayer");
    if (!local_player.has<RenderPosition>()) {
        return;
    }
    camera.target = local_player.get<RenderPosition>().val;
    camera.position = local_player.get<RenderPosition>().val
        + raylib::Vector3{0, 10, 10};
}
