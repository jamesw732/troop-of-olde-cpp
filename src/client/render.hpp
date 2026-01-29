#pragma once
#include <iostream>

#include "flecs.h"
#include "raylib.h"
#include "rlgl.h"

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

inline void DrawMeshWire(const Mesh& mesh, const Vector3& pos, const Color& color)
{
    // vertices are stored as [x, y, z, x, y, z, ...]
    const float* v = mesh.vertices;
    const unsigned short* idx = mesh.indices;

    for (int i = 0; i < mesh.triangleCount; i++)
    {
        unsigned short a = idx[i*3 + 0];
        unsigned short b = idx[i*3 + 1];
        unsigned short c = idx[i*3 + 2];

        Vector3 A = { v[a*3 + 0], v[a*3 + 1], v[a*3 + 2] };
        Vector3 B = { v[b*3 + 0], v[b*3 + 1], v[b*3 + 2] };
        Vector3 C = { v[c*3 + 0], v[c*3 + 1], v[c*3 + 2] };

        DrawLine3D(A, B, color);
        DrawLine3D(B, C, color);
        DrawLine3D(C, A, color);
    }
}

inline flecs::system register_render_system(
    flecs::world& world,
    Camera3D& camera,
    flecs::entity phase)
{
    return world.system<RenderPosition, RenderRotation, Scale, Color, ModelType>()
        .kind(phase)
        .each([&camera] (
            flecs::entity e,
            RenderPosition& pos,
            RenderRotation& rot,
            Scale& scale,
            Color& color,
            ModelType& model_type)
        {
            BeginMode3D(camera);
                rlPushMatrix();
                    rlTranslatef(pos.val.x, pos.val.y, pos.val.z);
                    rlRotatef(rot.val.x, 1, 0, 0);
                    rlRotatef(rot.val.y, 0, 1, 0);
                    rlRotatef(rot.val.z, 0, 0, 1);
                    rlScalef(scale.val.x, scale.val.y, scale.val.z);
                    if (model_type.name == "cube") {
                        DrawCube({0, scale.val.y / 2, 0}, 1, 1, 1, color);
                        DrawCubeWires({0, scale.val.y / 2, 0}, 1, 1, 1, get_wire_color(color));
                    }
                    else if (model_type.name == "3d_quad") {
                        DrawTriangle3D(
                            {-1.0 / 2, 0, -1.0 / 2},
                            {-1.0 / 2, 0, 1.0 / 2},
                            {1.0 / 2, 0, -1.0 / 2},
                            color
                        );
                        DrawTriangle3D(
                            {1.0 / 2, 0, 1.0 / 2},
                            {1.0 / 2, 0, -1.0 / 2},
                            {-1.0 / 2, 0, 1.0 / 2},
                            color
                        );
                    }
                    else if (model_type.name == "mesh") {
                        ModelPointer model = e.get<ModelPointer>();
                        DrawModel(*model.model, {}, 1, color);
                        DrawMeshWire(*model.model->meshes, {}, BLACK);
                    }
                rlPopMatrix();
            EndMode3D();
        }
    );
}
