#pragma once
#include <iostream>

#include "raylib-cpp.hpp"

#include "shared/components/physics.hpp"
#include "shared/components/movement.hpp"
#include "shared/components/ticks.hpp"


inline void movement_system(Position& pos, ClientMoveTick& ack_tick, MovementInputPacket& packet) {
    /*
     * pos: Position of this character
     * ack_tick: the previously acknowledged tick
     * packet: the received movement data, containing a tick and a sequence of MovementInputs
     */
    uint16_t start_tick = packet.tick - (packet.inputs.size() - 1);
    std::cout << "Processing " << packet.tick - ack_tick.val << " movement inputs" << std::endl;
    int dif = (int) (ack_tick.val - start_tick);
    // Precondition: received tick is at most 1 more than stored tick
    // If not satisfied, process whole buffer and hope for the best
    int start_idx = std::max(0, dif + 1);
    for (std::vector<MovementInput>::iterator it = packet.inputs.begin() + start_idx;
            it != packet.inputs.end(); ++it) {
        auto input = *it;
        raylib::Vector3 velocity((float) input.x, 0, (float) input.z);
        velocity = velocity.Normalize();
        velocity = velocity * 0.25;

        pos.val += velocity;
    }
    ack_tick.val = packet.tick;
}
