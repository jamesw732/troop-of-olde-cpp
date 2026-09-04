#pragma once
#include <cstdint>

enum class PacketType : uint8_t {
    MovementInputPacket,
    LoginRequestPacket,
    LoginResponsePacket,
    PlayerSpawnPacket,
    MovementUpdateBatchPacket,
    DisconnectPacket
};

