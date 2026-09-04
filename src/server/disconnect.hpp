#pragma once
#include <unordered_map>

#include "flecs.h"

#include "components.hpp"
#include "network.hpp"
#include "../shared/components.hpp"
#include "../shared/packets.hpp"
#include "../shared/serialize.hpp"

inline void register_disconnect_system(
    flecs::world& world,
    Network& network,
    std::unordered_map<uint32_t, flecs::entity>& client_id_to_entity
) {
    world.system<ClientId>()
        .with<Disconnected>()
        .each([&] (flecs::entity entity, ClientId& client_id) {
            DisconnectPacket dc_packet{client_id.id};
            world.query<ClientId, Connected>()
                .each([&] (ClientId& tgt_client_id, Connected) {
                    if (client_id.id == tgt_client_id.id) {
                        return;
                    }
                    std::cout << "Sending disconnect packet" << '\n';
                    auto [buffer, size] = serialize(dc_packet);
                    // Create packet and send to client
                    network.queue_data_reliable(tgt_client_id.id, buffer, size);
                }
            );
            client_id_to_entity.erase(client_id.id);
            entity.destruct();
        }
    );
}
