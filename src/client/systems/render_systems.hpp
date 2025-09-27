#pragma once
#include "enet.h"
#include "flecs.h"
#include "raylib-cpp.hpp"

#include "client/systems/render_system.hpp"
#include "shared/components/physics.hpp"


inline flecs::system register_render_system(flecs::world& world, raylib::Camera3D& camera) {
    return world.system<Position>("RenderSystem")
        .each([&camera](Position& pos) {
                render_system(camera, pos);
            }
        );
}
