#pragma once
#include "enet.h"
#include "flecs.h"
#include "raylib-cpp.hpp"

#include "shared/components/physics.hpp"


inline flecs::system register_render_system(flecs::world& world, raylib::Camera3D& camera) {
    return world.system<Position>("RenderSystem")
        .each([&camera](Position& pos) {
            BeginMode3D(camera);
                DrawCube(pos.val, 2.0f, 2.0f, 2.0f, RED);
                DrawCubeWires(pos.val, 2.0f, 2.0f, 2.0f, MAROON);
            EndMode3D();
            }
        );
}
