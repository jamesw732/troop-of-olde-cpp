#pragma once
#include <random>

#include "enet.h"

#include "client/input_buffer.hpp"
#include "shared/components/inputs.hpp"
#include "shared/serialize/helpers.hpp"
#include "shared/serialize/serialize_input_packet.hpp"


inline void movement_networking_system(ENetPeer* peer, InputBuffer& input_buffer, uint16_t& tick) {
    // Construct movement input packet from input buffer and send to server
    InputPacket input_data;
    input_data.tick = tick;
    for (MovementInput input: input_buffer.buffer) {
        input_data.inputs.push_back(input);
    }
    Buffer data_buffer;
    size_t size = bitsery::quickSerialization(OutputAdapter{data_buffer}, input_data);
    ENetPacket* packet = enet_packet_create(data_buffer.data(), size, 0);
    enet_peer_send(peer, 0, packet);

    // Test with networking conditions
    // std::random_device rd;
    // std::mt19937 gen(rd());
    // std::uniform_int_distribution<> distrib(1, 100);
    // int randomNumber = distrib(gen);
    // if (randomNumber < 50) {
    //     enet_peer_send(peer, 0, packet);
    // }
    // Test serialize
    // InputPacket res;
    // auto state = bitsery::quickDeserialization(
    //     InputAdapter{
    //         data_buffer.data(),
    //         size
    //     },
    //     res
    // );
}
