#pragma once
#include <unordered_map>
#include <iostream>

#include "flecs.h"

#include "shared/components.hpp"
#include "client/components.hpp"

inline void register_disconnect_system(flecs::world& world) {
    world.system<NetworkId, const Disconnected>()
        .each([&world] (flecs::entity entity, NetworkId& network_id, const Disconnected&) {
            std::cout << "Removing disconnected player" << '\n';
            auto netid_to_entity = world.get<NetworkMap>().netid_to_entity;
            netid_to_entity.erase(network_id);
            entity.destruct();
        }
    );
}
