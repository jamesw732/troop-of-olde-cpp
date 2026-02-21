#pragma once
#include <cstdint>
#include <vector>

#include "components.hpp"
#include "const.hpp"
#include "util.hpp"

enum class PacketType : uint8_t {
    MovementInputPacket,
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
    uint16_t size = 0;
    std::array<MovementInput, MAX_INPUT_BUFFER> inputs{};
};

inline std::ostream& operator<<(std::ostream& os, const MovementInputPacket& pkt) {
    os << "MovementInputPacket{\n";
    os << indent();
    print_indent(os);
    os << "tick: " << pkt.tick << "\n";
    print_indent(os);
    os << "inputs: ";
    print_array(os, pkt.inputs, pkt.size);
    os << unindent();
    print_indent(os);
    os <<"}\n";
    return os;
}

struct ClientLoginPacket {
    static constexpr PacketType id = PacketType::ClientLoginPacket;
    std::string name;
    Vector3 pos;
    Vector3 rot;
};

inline std::ostream& operator<<(std::ostream& os, const ClientLoginPacket& pkt) {
    os << "ClientLoginPacket{\n";
    os << indent();
    print_indent(os);
    os << "name: " << pkt.name << "\n";
    print_indent(os);
    os << "pos: " << pkt.pos << "\n";
    print_indent(os);
    os << "rot: " << pkt.rot << "\n";
    os << unindent();
    os <<"}\n";
    return os;
}

struct SpawnBatchPacket {
    static constexpr PacketType id = PacketType::SpawnBatchPacket;
    uint32_t local_player_id;
    std::vector<PlayerSpawnState> spawn_states;
};

inline std::ostream& operator<<(std::ostream& os, const SpawnBatchPacket& pkt) {
    os << "SpawnBatchPacket{\n";
    os << indent();
    print_indent(os);
    os << "local_player_id: " << pkt.local_player_id << "\n";
    print_indent(os);
    os << "spawn_states: ";
    print_vector(os, pkt.spawn_states);
    os << unindent();
    os <<"}\n";
    return os;
}

struct PlayerSpawnPacket {
    static constexpr PacketType id = PacketType::PlayerSpawnPacket;
    PlayerSpawnState spawn_state;
};

inline std::ostream& operator<<(std::ostream& os, const PlayerSpawnPacket& pkt) {
    os << "PlayerSpawnPacket{\n";
    os << indent();
    print_indent(os);
    os << "spawn_state: " << pkt.spawn_state << "\n";
    os << unindent();
    os << "}\n";
    return os;
}

struct MovementUpdateBatchPacket {
    static constexpr PacketType id = PacketType::MovementUpdateBatchPacket;
    std::vector<MovementUpdate> move_updates;
};

inline std::ostream& operator<<(std::ostream& os, const MovementUpdateBatchPacket& pkt) {
    os << "MovementUpdateBatchPacket{\n";
    os << indent();
    print_indent(os);
    os << "move_updates: ";
    print_vector(os, pkt.move_updates);
    os << unindent();
    os << "}\n";
    return os;
}

struct DisconnectPacket {
    static constexpr PacketType id = PacketType::DisconnectPacket;
    uint32_t network_id;
};

inline std::ostream& operator<<(std::ostream& os, const DisconnectPacket& pkt) {
    os << "DisconnectPacket{\n";
    os << indent();
    print_indent(os);
    os << "network_id: " << pkt.network_id << "\n";
    os << unindent();
    os << "}\n";
    return os;
}
