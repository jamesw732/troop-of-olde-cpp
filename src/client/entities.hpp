#include "flecs.h"

#include "components.hpp"
#include "../shared/animation_components.hpp"
#include "../shared/components.hpp"
#include "../shared/packets.hpp"

void add_character_components(flecs::entity entity);

inline flecs::entity create_local_player(flecs::world world) {
    auto entity = world.entity("LocalPlayer");
    entity.add<LocalPlayer>();

    // CAMERA
    entity.add<CamRotation>();
    entity.add<CamDistance>();
    entity.add<MouseRotationY>();

    add_character_components(entity);

    return entity;
}

inline flecs::entity create_remote_player(flecs::world world) {
    auto entity = world.entity();

    add_character_components(entity);

    return entity;
}

inline void add_character_components(flecs::entity entity) {
    entity.add<Character>();
    entity.add<DisplayName>();

    // NETWORKING
    entity.add<NetworkId>();
    entity.add<AckTick>();
    entity.add<RecvAckTick>();

    // MOVEMENT
    entity.add<SimPosition>();
    entity.add<SimRotation>();
    entity.add<SimGrounded>();
    entity.add<SimGravity>();

    entity.add<PredPosition>();
    entity.add<PredRotation>();
    entity.add<PredGrounded>();
    entity.add<PredGravity>();

    entity.add<PrevPredPosition>();
    entity.add<PrevPredRotation>();

    entity.add<RecvPosition>();
    entity.add<RecvRotation>();
    entity.add<RecvGrounded>();
    entity.add<RecvGravity>();

    // RENDERING
    entity.add<ModelPointer>();

    entity.add<LerpTimer>();
    entity.add<RenderPosition>();
    entity.add<RenderRotation>();
    entity.add<Scale>();
    entity.add<Color>();

    // ANIMATION
    entity.add<ModelAnimations>();

    entity.add<CurLocomotionState>();
    entity.add<RecvLocomotionState>();

    entity.add<AnimationFrame>();

    entity.add<LocomotionBlendFactor>();

    entity.add<RenderPose>();
    entity.add<CurLocomotionPose>();
    entity.add<PrevLocomotionPose>();
}
