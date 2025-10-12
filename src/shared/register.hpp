#include "flecs.h"

#include "shared/components.hpp"
#include "shared/packets.hpp"

inline void register_components(flecs::world world) {
    // Add normal components
    world.component<DisplayName>();
    world.component<NetworkId>();
    world.component<MovementInput>();
    world.component<Position>();
    world.component<ClientMoveTick>();
    world.component<PlayerSpawnState>();
    world.component<MovementUpdate>();
    // Add packets
    world.component<ClientLoginPacket>();
    world.component<SpawnBatchPacket>();
    world.component<PlayerSpawnPacket>();
    world.component<MovementUpdateBatchPacket>();
    world.component<MovementInputPacket>();
    // Set singleton components
    world.set<MovementUpdateBatchPacket>({});
}
