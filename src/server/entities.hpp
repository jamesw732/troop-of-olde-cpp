#include <iostream>

#include "flecs.h"

#include "server/components.hpp"
#include "shared/components.hpp"
#include "shared/packets.hpp"

inline flecs::entity create_character(flecs::world world) {
    auto entity = world.entity();
    std::cout << "Created character with entity id " << entity.id() << '\n';
    entity.add<Position>();
    entity.add<MovementInputPacket>();
    entity.add<Connection>();
    entity.add<ClientMoveTick>();
    entity.add<DisplayName>();
    entity.add<NetworkId>();
    entity.add<NeedsSpawnBroadcast>();
    return entity;
}
