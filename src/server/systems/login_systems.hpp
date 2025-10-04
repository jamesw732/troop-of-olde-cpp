#include "enet.h"
#include "flecs.h"

#include "server/components/networking.hpp"
#include "shared/components/network.hpp"
#include "shared/components/packets.hpp"
#include "shared/serialize/serialize.hpp"

inline void register_login_system(flecs::world world) {
    world.system<Connection, NetworkId>()
        .with<ClientLoginPacket>() // Move this to system/lambda when it has data
        .each([] (flecs::entity e, Connection& conn, NetworkId& network_id) {
            // Set character components locally (TODO)
            // Instruct client to create character
            PlayerSpawnPacket spawn{network_id};
            auto [buffer, size] = serialize(spawn);
            ENetPacket* packet = enet_packet_create(buffer.data(), size, 0);
            enet_peer_send(conn.peer, 1, packet);
            // Remove login packet from entity
            e.remove<ClientLoginPacket>();
            }
        );
}
