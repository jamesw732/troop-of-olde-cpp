#pragma once
#include <unordered_map>

#include "flecs.h"

#include "shared/components.hpp"

inline void register_disconnect_system(flecs::world& world) {
    world.system<NetworkId>()
        .with<Disconnected>()
        .each([] (flecs::entity entity, NetworkId& network_id) {
            entity.destruct();
        }
    );
}
