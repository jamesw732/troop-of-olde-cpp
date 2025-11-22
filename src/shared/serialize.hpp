#pragma once
#include <iostream>
#include <tuple>

#include "flecs.h"

#include "const.hpp"
#include "components.hpp"
#include "packets.hpp"
#include "util.hpp"

static_assert(sizeof(float) == 4, "Type float must be 4 bytes");

/*
 * Main serialize function
 */
template<typename T>
inline std::tuple<Buffer, size_t> serialize(const T& obj) {
    Buffer buffer;
    bitsery::Serializer<OutputAdapter> ser{OutputAdapter{buffer}};
    // std::cout << (int) T::id << '\n';
    ser.value1b(T::id);
    ser.object(obj);
    ser.adapter().flush();
    size_t size = ser.adapter().writtenBytesCount();

    return std::tuple<Buffer, size_t>{buffer, size};
}

/*
 * Test deserialize function
 */
template<typename T>
T deserialize(const uint8_t* buffer, size_t size) {
    bitsery::Deserializer<InputAdapter> des{InputAdapter{buffer, size}};
    PacketType pkt_type;
    std::cout << "Packet type: " << (int) pkt_type << std::endl;
    des.value1b(pkt_type);
    T obj;
    des.object(obj);
    return obj;
}

// Packets
template<typename S>
void serialize(S& s, ClientLoginPacket& login) {
    s.text1b(login.name, 32);
    s.object(login.pos);
    s.object(login.rot);
}

template<typename S>
void serialize(S& s, SpawnBatchPacket& spawns) {
    s.value4b(spawns.local_player_id);
    s.container(spawns.spawn_states, MAX_CLIENTS);
}

template<typename S>
void serialize(S& s, MovementInputPacket& input_packet) {
    s.value2b(input_packet.tick);
    s.object(input_packet.inputs);
}

template<typename S>
void serialize(S& s, PlayerSpawnPacket& spawn_packet) {
    s.object(spawn_packet.spawn_state);
}

template<typename S>
void serialize(S& s, MovementUpdateBatchPacket& batch) {
    s.container(batch.move_updates, MAX_CLIENTS);
}

template<typename S>
void serialize(S& s, DisconnectPacket& dc_packet) {
    s.value4b(dc_packet.network_id);
}

// Primitives
template <typename S>
void serialize (S& s, raylib::Vector3& v) {
    s.value4b(v.x);
    s.value4b(v.y);
    s.value4b(v.z);
}

// Components
template<typename S>
void serialize(S& s, PlayerSpawnState& spawn_state) {
    s.value4b(spawn_state.network_id);
    s.text1b(spawn_state.name, 32);
    s.object(spawn_state.pos);
    s.object(spawn_state.rot);
}

template <typename S>
void serialize(S& s, std::vector<MovementInput>& v) {
    s.container(v, 100, [](S& s, MovementInput& input) {
        s.value1b(input.x);
        s.value1b(input.z);
        s.value1b(input.rot_y);
        s.value2b(input.mouse_rot_y);
        s.value1b(input.jump);
    });
}

template<typename S>
void serialize(S& s, MovementUpdate& move_update) {
    s.value4b(move_update.network_id);
    s.value2b(move_update.ack_tick);
    s.object(move_update.pos);
    s.object(move_update.rot);
    s.value4b(move_update.gravity);
    s.value1b(move_update.grounded);
}
