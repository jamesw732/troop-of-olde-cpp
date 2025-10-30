#pragma once
#include <iostream>

#include "flecs.h"
#include "raylib-cpp.hpp"

#include "../shared/components.hpp"


inline flecs::system register_render_system(flecs::world& world, raylib::Camera3D& camera, flecs::entity phase) {
    return world.system<RenderPosition, RenderRotation>("RenderSystem")
        .kind(phase)
        .each([&camera](RenderPosition& pos, RenderRotation& rot) {
            BeginMode3D(camera);
                rlPushMatrix();
                    rlTranslatef(pos.val.x, pos.val.y, pos.val.z);
                    rlRotatef(rot.val, 0, 1, 0);
                    DrawCube({0, 0, 0}, 2.0f, 2.0f, 2.0f, RED);
                    DrawCubeWires({0, 0, 0}, 2.0f, 2.0f, 2.0f, MAROON);
                rlPopMatrix();
            EndMode3D();
            }
        );
}
