#pragma once
#include <iostream>
#include <tuple>

#include "flecs.h"
#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/traits/vector.h>
#include <bitsery/traits/string.h>
#include <bitsery/traits/array.h>

#include "shared/util.hpp"
#include "shared/components.hpp"
#include "shared/packets.hpp"

using Buffer = std::vector<uint8_t>;
using OutputAdapter = bitsery::OutputBufferAdapter<Buffer>;
using InputAdapter = bitsery::InputBufferAdapter<uint8_t*>;

static_assert(sizeof(float) == 4, "Floats must be 4 bytes");


template<typename T>
inline std::tuple<Buffer, size_t> serialize(const T& obj) {
    Buffer buffer;
    bitsery::Serializer<OutputAdapter> ser{OutputAdapter{buffer}};
    ser.value1b(T::id);
    ser.object(obj);
    ser.adapter().flush();
    size_t size = ser.adapter().writtenBytesCount();

    return std::tuple<Buffer, size_t>{buffer, size};
}

inline void handle_packet(flecs::entity entity, const uint8_t* buffer, size_t size) {
    bitsery::Deserializer<InputAdapter> des{InputAdapter{buffer, size}};
    PacketType pkt_type;
    des.value1b(pkt_type);
    switch (pkt_type) {
        case PacketType::MovementInput: {
            MovementInputPacket input_packet;
            des.object(input_packet);
            entity.set<MovementInputPacket>(input_packet);

            MovementInput input = input_packet.inputs.back();
            uint16_t tick = input_packet.tick;
            std::cout << "Received "
                << input_packet.inputs.size()
                << " movement inputs up to tick "
                << tick
                << ". Most recent: "
                << (int) input.x
                << ", "
                << (int) input.z
                << std::endl;
            break;
        }

        case PacketType::MovementUpdate: {
            auto* move_update = entity.try_get_mut<MovementUpdatePacket>();
            if (!move_update) {
                return;
            }
            // MovementUpdatePacket& move_update = entity.get_mut<MovementUpdatePacket>();
            des.object(*move_update);

            std::cout << "Received position "
                << vector3_to_string(move_update->pos)
                << " from server for tick "
                << (int) move_update->ack_tick
                << std::endl;
            break;
        }

        case PacketType::ClientLoginPacket: {
            std::cout << "Received login packet" << std::endl;
            entity.add<ClientLoginPacket>();
            ClientLoginPacket& login = entity.get_mut<ClientLoginPacket>();
            des.object(login);
            break;
        }

        case PacketType::PlayerSpawnPacket: {
            std::cout << "Received spawn packet" << std::endl;
            entity.add<PlayerSpawnPacket>();
            PlayerSpawnPacket& spawn = entity.get_mut<PlayerSpawnPacket>();
            des.object(spawn);
            break;
        }
    }
}


template <typename S>
void serialize (S& s, raylib::Vector3& v) {
    s.value4b(v.x);
    s.value4b(v.y);
    s.value4b(v.z);
}

template <typename S>
void serialize(S& s, Position& pos) {
    serialize(s, pos.val);
}

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

template <typename S>
void serialize(S& s, std::vector<MovementInput>& v) {
    s.container(v, 500, [](S& s, MovementInput& input) {
        s.value1b(input.x);
        s.value1b(input.z);
    });
}

template<typename S>
void serialize(S& s, MovementInputPacket& input_packet) {
    s.value2b(input_packet.tick);
    serialize(s, input_packet.inputs);
}

template<typename S>
void serialize(S& s, MovementUpdatePacket& move_update) {
    s.value2b(move_update.ack_tick);
    serialize(s, move_update.pos);
}

template<typename S>
void serialize(S& s, DisplayName name) {
    s.text1b(name.name, 32);
}

