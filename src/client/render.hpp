#pragma once
#include <iostream>

#include "flecs.h"
#include "raylib.h"
#include "rlgl.h"

#include "../shared/components.hpp"
#include "../shared/const.hpp"
#include "components.hpp"

inline Color get_wire_color(Color base_color) {
    float brightness = 0.299f * base_color.r + 0.587f * base_color.g + 0.114f * base_color.b;
    float factor = (brightness < 128) ? 1.5f : 0.5f;
    Color wire_color = {
        (unsigned char)fminf(base_color.r * factor, 255.0f),
        (unsigned char)fminf(base_color.g * factor, 255.0f),
        (unsigned char)fminf(base_color.b * factor, 255.0f),
        base_color.a
    };
    return wire_color;

}

inline void RenderModel(Model model, Vector3 pos, Vector3 rot, Vector3 scale, Color color) {
        rlPushMatrix();
            rlTranslatef(pos.x, pos.y, pos.z);
            rlRotatef(rot.x, 1, 0, 0);
            rlRotatef(rot.y, 0, 1, 0);
            rlRotatef(rot.z, 0, 0, 1);
            rlScalef(scale.x, scale.y, scale.z);
            DrawModel(model, {}, 1, color);
        rlPopMatrix();
}

inline void RenderModelWires(Model model, Vector3 pos, Vector3 rot, Vector3 scale, Color color) {
        rlPushMatrix();
            rlTranslatef(pos.x, pos.y, pos.z);
            rlRotatef(rot.x, 1, 0, 0);
            rlRotatef(rot.y, 0, 1, 0);
            rlRotatef(rot.z, 0, 0, 1);
            rlScalef(scale.x, scale.y, scale.z);
            DrawModelWires(model, {}, 1, get_wire_color(color));
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
                RenderModelWires(*model.model, pos.val, rot.val, scale.val, color);
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
            float render_frame = fmodf(frame.frame, anim.keyframeCount - 2);
            if (alpha.val < 1.0) {
                UpdateModelAnimationEx(
                    *model.model,
                    prev_anim, prev_frame.frame,
                    anim, render_frame,
                    alpha.val
                );
            }
            else {
                UpdateModelAnimation(*model.model, anim, render_frame);
            }
            BeginMode3D(camera);
                RenderModel(*model.model, pos.val, rot.val, scale.val, color);
            EndMode3D();
        }
    );
}
