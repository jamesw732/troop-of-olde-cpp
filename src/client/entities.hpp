#include "flecs.h"

#include "components.hpp"
#include "../shared/components.hpp"
#include "../shared/packets.hpp"

inline flecs::entity create_local_player(flecs::world world) {
    auto entity = world.entity("LocalPlayer");
    entity.add<LocalPlayer>();
    entity.add<Position>();
    entity.add<DisplayName>();
    entity.add<NetworkId>();
    entity.add<TargetPosition>();
    entity.add<PrevPosition>();
    entity.add<LerpTimer>();
    entity.add<MovementInput>();
    entity.add<AckTick>();
    return entity;
}

inline flecs::entity create_remote_player(flecs::world world) {
    auto entity = world.entity();
    entity.add<Position>();
    entity.add<DisplayName>();
    entity.add<NetworkId>();
    entity.add<TargetPosition>();
    entity.add<PrevPosition>();
    entity.add<LerpTimer>();
    entity.add<AckTick>();
    return entity;
}
