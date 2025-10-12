#pragma once
#include <unordered_map>
#include <iostream>

#include "flecs.h"

#include "shared/components.hpp"

inline void register_disconnect_system(
    flecs::world& world,
    std::unordered_map<NetworkId, flecs::entity>& netid_to_entity) {
    world.system<NetworkId, const Disconnected>()
        .each([&netid_to_entity] (flecs::entity entity, NetworkId& network_id, const Disconnected&) {
            std::cout << "Removing disconnected player" << '\n';
            netid_to_entity.erase(network_id);
            entity.destruct();
        }
    );
}
