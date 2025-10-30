#pragma once
#include <iostream>

#include "flecs.h"

#include "components.hpp"
#include "../shared/components.hpp"
#include "../shared/packets.hpp"


inline flecs::entity add_character_components(flecs::entity entity) {
    entity.add<Position>();
    entity.add<Rotation>();
    entity.add<MovementInputPacket>();
    entity.add<Connected>();
    entity.add<ClientMoveTick>();
    entity.add<DisplayName>();
    entity.add<NetworkId>();
    entity.add<NeedsSpawnBroadcast>();
    return entity;
}
