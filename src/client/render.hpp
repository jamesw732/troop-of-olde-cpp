#pragma once
#include <iostream>

#include "flecs.h"
#include "raylib.h"
#include "rlgl.h"

#include "../shared/components.hpp"
#include "../shared/const.hpp"
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

inline void RenderModel(Model model, Vector3 pos, Vector3 rot, Vector3 scale, Color color) {
        rlPushMatrix();
            rlTranslatef(pos.x, pos.y, pos.z);
            rlRotatef(rot.x, 1, 0, 0);
            rlRotatef(rot.y, 0, 1, 0);
            rlRotatef(rot.z, 0, 0, 1);
            rlScalef(scale.x, scale.y, scale.z);
            DrawModel(model, {}, 1, color);
            DrawModelWires(model, {}, 1, BLACK);
        rlPopMatrix();
}

inline flecs::system register_render_system(
    flecs::world& world,
    Camera3D& camera,
    flecs::entity phase)
{
    return world.system<ModelPointer, RenderPosition, RenderRotation, Scale, Color>()
        .kind(phase)
        .without<ModelAnimations>()
        .each([&camera] (
            const ModelPointer model,
            const RenderPosition pos,
            const RenderRotation rot,
            const Scale scale,
            const Color color
            )
        {
            BeginMode3D(camera);
                RenderModel(*model.model, pos.val, rot.val, scale.val, color);
            EndMode3D();
        }
    );
}

inline flecs::system register_animation_render_system(
    flecs::world& world,
    Camera3D& camera,
    flecs::entity phase)
{
    return world.system<ModelPointer, ModelAnimations,
                        CurLocomotionState, AnimationFrame,
                        PrevLocomotionState, PrevAnimationFrame,
                        BlendFactor,
                        RenderPosition, RenderRotation, Scale, Color>()
        .kind(phase)
        .each([&camera] (
            const ModelPointer model,
            const ModelAnimations anims,
            const CurLocomotionState movement_state,
            const AnimationFrame frame,
            const PrevLocomotionState prev_movement_state,
            const PrevAnimationFrame prev_frame,
            const BlendFactor alpha,
            const RenderPosition pos,
            const RenderRotation rot,
            const Scale scale,
            const Color color
            )
        {
            // TODO: implement half-body animations, once needed
            std::string anim_name = anim_names[(size_t) movement_state.state];
            std::string prev_anim_name = anim_names[(size_t) prev_movement_state.state];
            ModelAnimation anim = anims.map->at(anim_name);
            ModelAnimation prev_anim = anims.map->at(prev_anim_name);
            UpdateModelAnimationEx(
                *model.model,
                prev_anim, prev_frame.frame,
                anim, frame.frame,
                alpha.val
            );
            BeginMode3D(camera);
                RenderModel(*model.model, pos.val, rot.val, scale.val, color);
            EndMode3D();
        }
    );
}
