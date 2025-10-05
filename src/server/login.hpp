#include "enet.h"
#include "flecs.h"

#include "server/components.hpp"
#include "shared/packets.hpp"
#include "shared/serialize.hpp"

inline void register_batch_login_system(flecs::world& world) {
    world.system<NetworkId, Position>()
        .each([] (NetworkId id, Position pos) {
            }
         );
}

inline void register_login_system(flecs::world& world) {
    world.system<Connection, NetworkId>()
        .with<ClientLoginPacket>() // Move this to system/lambda when it has data
        .each([] (flecs::entity e, Connection& conn, NetworkId& network_id) {
            // Set character components locally (TODO)
            // Instruct client to create character
            PlayerSpawnPacket spawn{network_id};
            auto [buffer, size] = serialize(spawn);
            ENetPacket* packet = enet_packet_create(buffer.data(), size, ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(conn.peer, 1, packet);
            // Remove login packet from entity
            e.remove<ClientLoginPacket>();
            }
        );
}
