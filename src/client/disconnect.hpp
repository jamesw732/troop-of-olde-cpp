#pragma once
#include <unordered_map>
#include <iostream>

#include "flecs.h"

#include "components.hpp"
#include "../shared/network-components.hpp"

inline void register_disconnect_system(flecs::world& world, auto& netid_to_entity) {
    world.system<NetworkId, const Disconnected>()
        .each([&] (flecs::entity entity, NetworkId& network_id, const Disconnected&) {
            std::cout << "Removing disconnected player" << '\n';
            netid_to_entity.erase(network_id.id);
            entity.destruct();
        }
    );
}
