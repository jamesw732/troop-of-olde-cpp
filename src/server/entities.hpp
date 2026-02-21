#pragma once
#include <iostream>

#include "flecs.h"

#include "components.hpp"
#include "../shared/components.hpp"
#include "../shared/packets.hpp"


inline flecs::entity add_character_components(flecs::entity entity) {
    entity.add<Character>();
    entity.add<DisplayName>();

    entity.add<NetworkId>();
    entity.add<Connected>();
    entity.add<NeedsSpawnBroadcast>();

    entity.add<PredPosition>();
    entity.add<PredRotation>();
    entity.add<PredGravity>();
    entity.add<PredGrounded>();

    entity.add<SimPosition>();
    entity.add<SimRotation>();
    entity.add<SimGravity>();
    entity.add<SimGrounded>();
    entity.add<Scale>();

    entity.add<CurMoveTick>();
    entity.add<RecvMoveTick>();
    entity.add<InputBuffer>();

    return entity;
}
