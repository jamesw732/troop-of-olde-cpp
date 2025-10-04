#pragma once

#include "shared/components/packets.hpp"
#include "shared/serialize/helpers.hpp"
#include "shared/serialize/serialize_names.hpp"

template<typename S>
void serialize(S& s, ClientLoginPacket login) {
    s.object(login.name);
}

template<typename S>
void serialize(S& s, PlayerSpawnPacket spawn) {
    s.object(spawn.network_id);
    s.object(spawn.name);
}

template<typename S>
void serialize(S& s, NetworkId network_id) {
    s.value4b(network_id.id);
}
