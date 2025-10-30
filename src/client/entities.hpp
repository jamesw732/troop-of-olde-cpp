#include "flecs.h"

#include "components.hpp"
#include "../shared/components.hpp"
#include "../shared/packets.hpp"

inline flecs::entity create_local_player(flecs::world world) {
    auto entity = world.entity("LocalPlayer");
    entity.add<LocalPlayer>();
    entity.add<DisplayName>();
    entity.add<NetworkId>();
    entity.add<RenderPosition>();
    entity.add<SimPosition>();
    entity.add<PrevSimPosition>();
    entity.add<RenderRotation>();
    entity.add<SimRotation>();
    entity.add<PrevSimRotation>();
    entity.add<LerpTimer>();
    entity.add<MovementInput>();
    entity.add<AckTick>();
    return entity;
}

inline flecs::entity create_remote_player(flecs::world world) {
    auto entity = world.entity();
    entity.add<RenderPosition>();
    entity.add<SimPosition>();
    entity.add<PrevSimPosition>();
    entity.add<RenderRotation>();
    entity.add<SimRotation>();
    entity.add<PrevSimRotation>();
    entity.add<DisplayName>();
    entity.add<NetworkId>();
    entity.add<LerpTimer>();
    entity.add<AckTick>();
    return entity;
}
