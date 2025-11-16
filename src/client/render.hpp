#pragma once
#include <iostream>

#include "flecs.h"
#include "raylib-cpp.hpp"

#include "../shared/components.hpp"
#include "components.hpp"

inline Color get_wire_color(Color cube_color) {
    float brightness = 0.299f * cube_color.r + 0.587f * cube_color.g + 0.114f * cube_color.b;
    float factor = (brightness < 128) ? 1.5f : 0.5f;
    Color wire_color = {
        (unsigned char)fminf(cube_color.r * factor, 255.0f),
        (unsigned char)fminf(cube_color.g * factor, 255.0f),
        (unsigned char)fminf(cube_color.b * factor, 255.0f),
        cube_color.a
    };
    return wire_color;

}

inline flecs::system register_terrain_render_system(
    flecs::world& world,
    raylib::Camera3D& camera,
    flecs::entity phase)
{
    return world.system<SimPosition, SimRotation, Scale, Color, ModelName, Terrain>()
        .kind(phase)
        .each([&camera](SimPosition& pos,
                    SimRotation& rot,
                    Scale& scale,
                    Color& color,
                    ModelName& model_name,
                    Terrain)
        {
            BeginMode3D(camera);
                rlPushMatrix();
                    rlTranslatef(pos.val.x, pos.val.y, pos.val.z);
                    rlRotatef(rot.val.x, 1, 0, 0);
                    rlRotatef(rot.val.y, 0, 1, 0);
                    rlRotatef(rot.val.z, 0, 0, 1);
                    if (model_name.name == "3d_quad") {
                        DrawTriangle3D(
                            {-scale.val.x / 2, 0, -scale.val.z / 2},
                            {-scale.val.x / 2, 0, scale.val.z / 2},
                            {scale.val.x / 2, 0, -scale.val.z / 2},
                            color
                        );
                        DrawTriangle3D(
                            {scale.val.x / 2, 0, scale.val.z / 2},
                            {scale.val.x / 2, 0, -scale.val.z / 2},
                            {-scale.val.x / 2, 0, scale.val.z / 2},
                            color
                        );
                    }
                rlPopMatrix();
            EndMode3D();
        }
    );
}

inline flecs::system register_character_render_system(
    flecs::world& world,
    raylib::Camera3D& camera,
    flecs::entity phase)
{
    return world.system<RenderPosition, RenderRotation, Scale, Color, ModelName, Character>()
        .kind(phase)
        .each([&camera](RenderPosition& pos,
                    RenderRotation& rot,
                    Scale& scale,
                    Color& color,
                    ModelName& model_name,
                    Character)
        {
            BeginMode3D(camera);
                rlPushMatrix();
                    rlTranslatef(pos.val.x, pos.val.y, pos.val.z);
                    rlRotatef(rot.val.y, 0, 1, 0);
                    DrawCube({0, scale.val.y / 2, 0}, scale.val.x, scale.val.y, scale.val.z, color);
                    DrawCubeWires({0, scale.val.y / 2, 0}, scale.val.x, scale.val.y, scale.val.z,
                        get_wire_color(color));
                rlPopMatrix();
            EndMode3D();
        }
    );
}
