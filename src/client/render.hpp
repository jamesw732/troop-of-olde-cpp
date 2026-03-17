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
    return world.system<ModelPointer, RenderPosition, RenderRotation, Scale, Color>()
        .kind(phase)
        .with<RenderOffset>().oper(flecs::Not)
        .each([&camera] (
            const ModelPointer model,
            const RenderPosition pos,
            const RenderRotation rot,
            const Scale scale,
            const Color color
            )
        {
            /* BoundingBox box = GetModelBoundingBox(*model.model); */
            /* std::cout << box.min << ", " << box.max << "\n"; */
            BeginMode3D(camera);
                rlPushMatrix();
                    rlTranslatef(pos.val.x, pos.val.y, pos.val.z);
                    rlRotatef(rot.val.x, 1, 0, 0);
                    rlRotatef(rot.val.y, 0, 1, 0);
                    rlRotatef(rot.val.z, 0, 0, 1);
                    rlScalef(scale.val.x, scale.val.y, scale.val.z);
                    DrawModel(*model.model, {}, 1, color);
                    DrawModelWires(*model.model, {}, 1, BLACK);
                    /* DrawMeshWire(*model.model->meshes, {}, BLACK); */
                    /* } */
                rlPopMatrix();
            EndMode3D();
        }
    );
}

inline flecs::system register_render_with_offset_system(
    flecs::world& world,
    Camera3D& camera,
    flecs::entity phase)
{
    return world.system<ModelPointer, RenderPosition, RenderRotation, Scale, Color, RenderOffset>()
        .kind(phase)
        .each([&camera] (
            const ModelPointer model,
            const RenderPosition pos,
            const RenderRotation rot,
            const Scale scale,
            const Color color,
            const RenderOffset offset
            )
        {
            /* BoundingBox box = GetModelBoundingBox(*model.model); */
            /* std::cout << box.min << ", " << box.max << "\n"; */
            
            BeginMode3D(camera);
                rlPushMatrix();
                    rlTranslatef(pos.val.x, pos.val.y, pos.val.z);
                    rlTranslatef(offset.val.x, offset.val.y, offset.val.z);
                    rlRotatef(rot.val.x, 1, 0, 0);
                    rlRotatef(rot.val.y, 0, 1, 0);
                    rlRotatef(rot.val.z, 0, 0, 1);
                    rlScalef(scale.val.x, scale.val.y, scale.val.z);
                    DrawModel(*model.model, {}, 1, color);
                    DrawModelWires(*model.model, {}, 1, BLACK);
                rlPopMatrix();
            EndMode3D();
        }
    );
}
