#include "flecs.h"

#include "components.hpp"
#include "../shared/animation_components.hpp"
#include "../shared/components.hpp"
#include "../shared/packets.hpp"

inline flecs::entity create_local_player(flecs::world world) {
    auto entity = world.entity("LocalPlayer");
    entity.add<Character>();
    entity.add<LocalPlayer>();
    entity.add<DisplayName>();

    entity.add<NetworkId>();
    entity.add<AckTick>();
    entity.add<RecvAckTick>();

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

    entity.add<LerpTimer>();
    entity.add<RenderPosition>();
    entity.add<RenderRotation>();

    entity.add<ModelPointer>();
    entity.add<Scale>();
    entity.add<Color>();

    entity.add<CamRotation>();
    entity.add<CamDistance>();

    entity.add<ModelAnimations>();
    entity.add<CurLocomotionState>();
    entity.add<PrevLocomotionState>();
    entity.add<RecvLocomotionState>();
    entity.add<AnimationFrame>();
    entity.add<PrevAnimationFrame>();
    entity.add<BlendFactor>();

    return entity;
}

inline flecs::entity create_remote_player(flecs::world world) {
    auto entity = world.entity();
    entity.add<Character>();
    entity.add<DisplayName>();

    entity.add<NetworkId>();
    entity.add<AckTick>();
    entity.add<RecvAckTick>();

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
    entity.add<RecvGravity>();
    entity.add<RecvGrounded>();

    entity.add<LerpTimer>();
    entity.add<RenderPosition>();
    entity.add<RenderRotation>();

    entity.add<ModelPointer>();
    entity.add<Scale>();
    entity.add<Color>();

    entity.add<ModelAnimations>();
    entity.add<CurLocomotionState>();
    entity.add<PrevLocomotionState>();
    entity.add<RecvLocomotionState>();
    entity.add<AnimationFrame>();
    entity.add<PrevAnimationFrame>();
    entity.add<BlendFactor>();

    return entity;
}
