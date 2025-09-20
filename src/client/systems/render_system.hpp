#pragma once
#include "raylib-cpp.hpp"

#include "shared/components/physics.hpp"

inline void render_system(Camera3D camera, Transformation& t) {
    BeginMode3D(camera);
        DrawCube(t.pos, 2.0f, 2.0f, 2.0f, RED);
        DrawCubeWires(t.pos, 2.0f, 2.0f, 2.0f, MAROON);
    EndMode3D();
}
