#pragma once
#include <cstdint>
#include <vector>

#include "raylib-cpp.hpp"

#include "shared/components/network.hpp"
#include "shared/components/names.hpp"
#include "movement.hpp"

enum class PacketType : uint8_t {
    MovementInput,
    MovementUpdate,
    ClientLoginPacket,
    PlayerSpawnPacket
};

/*
 * Contains client's  movement tick and all unacknowledged MovementInputs
 * This is serialized and sent to the server.
 */
struct MovementInputPacket {
    static constexpr PacketType id = PacketType::MovementInput;
    // The current movement tick, last movement tick in the buffer
    uint16_t tick = 0;
    // All movement inputs in the buffer
    std::vector<MovementInput> inputs;
};

/*
 * Contains the server's currently acknowledged tick and the corresponding position
 */
struct MovementUpdatePacket {
    static constexpr PacketType id = PacketType::MovementUpdate;
    uint16_t ack_tick = -1;
    raylib::Vector3 pos{0, 0, 0};
};

struct ClientLoginPacket {
    static constexpr PacketType id = PacketType::ClientLoginPacket;
    DisplayName name;
};

struct PlayerSpawnPacket {
    static constexpr PacketType id = PacketType::PlayerSpawnPacket;
    NetworkId network_id;
    DisplayName name;
};
