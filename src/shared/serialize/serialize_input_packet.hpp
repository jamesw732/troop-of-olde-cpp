#pragma once
#include "helpers.hpp"
#include "shared/components/inputs.hpp"

template <typename S>
void serialize(S& s, std::vector<MovementInput>& v) {
    s.container(v, 100, [](S& s, std::array<int8_t, 2>& arr) {
        s.container1b(arr);
    });

}

template<typename S>
void serialize(S& s, InputPacket& input_packet) {
    s.value2b(input_packet.tick);
    serialize(s, input_packet.inputs);
}
