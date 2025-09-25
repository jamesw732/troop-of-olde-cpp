#pragma once
#include "helpers.hpp"
#include "shared/components/movement.hpp"


template <typename S>
void serialize(S& s, std::vector<MovementInput>& v) {
    s.container(v, 100, [](S& s, MovementInput& input) {
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
void serialize(S& s, ServerMovementUpdate& move_update) {
    s.value2b(move_update.ack_tick);
    serialize(s, move_update.pos);
}
