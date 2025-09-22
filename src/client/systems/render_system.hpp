#pragma once
#include "raylib-cpp.hpp"

#include "shared/components/physics.hpp"

inline void render_system(Camera3D camera, Position& pos) {
    BeginMode3D(camera);
        DrawCube(pos.val, 2.0f, 2.0f, 2.0f, RED);
        DrawCubeWires(pos.val, 2.0f, 2.0f, 2.0f, MAROON);
    EndMode3D();
}
