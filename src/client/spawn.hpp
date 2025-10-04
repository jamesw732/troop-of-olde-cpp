#pragma once
#include <iostream>

#include "flecs.h"

#include "client/components.hpp"
#include "shared/components.hpp"
#include "shared/packets.hpp"


inline void register_player_spawn_system(flecs::world& world) {
    // Slide over current position to prev. Target will be mutated by other systems.
    world.system<PlayerSpawnPacket>()
        .each([&world] (flecs::entity e, PlayerSpawnPacket& spawn) {
                std::cout << "Spawning Character" << std::endl;
                e.add<Position>();
                e.add<TargetPosition>();
                e.add<PrevPosition>();
                e.add<LerpTimer>();
                e.add<MovementUpdatePacket>();
                e.add<MovementInput>();
                e.add<LocalPlayer>();
                e.set<NetworkId>(spawn.network_id);
                e.set<DisplayName>(spawn.name);
                e.remove<PlayerSpawnPacket>();
            }
        );
}
