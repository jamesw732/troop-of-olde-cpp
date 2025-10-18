#pragma once
#include <unordered_map>

#include "flecs.h"

#include "server/components.hpp"
#include "server/network.hpp"
#include "shared/components.hpp"
#include "shared/packets.hpp"
#include "shared/serialize.hpp"

inline void register_disconnect_system(flecs::world& world, Network& network) {
    world.system<NetworkId>()
        .with<Disconnected>()
        .each([&] (flecs::entity entity, NetworkId& network_id) {
            DisconnectPacket dc_packet{network_id};
            world.query<NetworkId, Connected>()
                .each([&] (NetworkId& tgt_network_id, Connected) {
                    if (network_id.id == tgt_network_id.id) {
                        return;
                    }
                    std::cout << "Sending disconnect packet" << '\n';
                    auto [buffer, size] = serialize(dc_packet);
                    // Create packet and send to client
                    network.queue_data_unreliable(tgt_network_id, buffer, size);
                }
            );
            entity.destruct();
        }
    );
}
