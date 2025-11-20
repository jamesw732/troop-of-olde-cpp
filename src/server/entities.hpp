#pragma once
#include <iostream>

#include "flecs.h"

#include "components.hpp"
#include "../shared/components.hpp"
#include "../shared/packets.hpp"


inline flecs::entity add_character_components(flecs::entity entity) {
    entity.add<Character>();
    entity.add<SimPosition>();
    entity.add<SimRotation>();
    entity.add<Scale>();
    entity.add<MovementInputPacket>();
    entity.add<Connected>();
    entity.add<ClientMoveTick>();
    entity.add<DisplayName>();
    entity.add<NetworkId>();
    entity.add<NeedsSpawnBroadcast>();

    entity.add<Gravity>();
    entity.add<Grounded>();
    return entity;
}
