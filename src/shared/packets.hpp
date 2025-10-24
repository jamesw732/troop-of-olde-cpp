#pragma once
#include <cstdint>
#include <vector>

#include "components.hpp"

enum class PacketType : uint8_t {
    MovementInputPacket,
    MovementUpdate,
    ClientLoginPacket,
    SpawnBatchPacket,
    PlayerSpawnPacket,
    MovementUpdateBatchPacket,
    DisconnectPacket
};

/*
 * Contains client's  movement tick and all unacknowledged MovementInputs
 * This is serialized and sent to the server.
 */
struct MovementInputPacket {
    static constexpr PacketType id = PacketType::MovementInputPacket;
    // The current movement tick, last movement tick in the buffer
    uint16_t tick = 0;
    // All movement inputs in the buffer
    std::vector<MovementInput> inputs;
};

struct ClientLoginPacket {
    static constexpr PacketType id = PacketType::ClientLoginPacket;
    DisplayName name;
};

struct SpawnBatchPacket {
    static constexpr PacketType id = PacketType::SpawnBatchPacket;
    NetworkId local_player_id;
    std::vector<PlayerSpawnState> spawn_states;
};

struct PlayerSpawnPacket {
    static constexpr PacketType id = PacketType::PlayerSpawnPacket;
    PlayerSpawnState spawn_state;
};

struct MovementUpdateBatchPacket {
    static constexpr PacketType id = PacketType::MovementUpdateBatchPacket;
    std::vector<MovementUpdate> move_updates;
};

struct DisconnectPacket {
    static constexpr PacketType id = PacketType::DisconnectPacket;
    NetworkId network_id;
};
