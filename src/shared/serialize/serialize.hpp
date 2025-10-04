#pragma once
#include <iostream>
#include <tuple>

#include "flecs.h"

#include "helpers.hpp"
#include "shared/components/packets.hpp"
#include "shared/serialize/serialize_login.hpp"
#include "shared/serialize/serialize_movement.hpp"
#include "shared/serialize/serialize_physics.hpp"
#include "shared/serialize/serialize_vector3.hpp"
#include "shared/util.hpp"


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
