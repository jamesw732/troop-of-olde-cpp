#include "flecs.h"

#include "shared/components.hpp"
#include "shared/packets.hpp"

inline void register_components(flecs::world world) {
    world.component<DisplayName>();
    world.component<NetworkId>();
    world.component<MovementInput>();
    world.component<Position>();
    world.component<ClientMoveTick>();
    world.component<PlayerSpawnState>();

    world.component<MovementInputPacket>();
    world.component<MovementUpdatePacket>();
    world.component<ClientLoginPacket>();
    world.component<SpawnBatchPacket>();
}
