#pragma once
#include <cstdint>
#include <queue>
#include <iostream>

#include "shared/components/physics.hpp"


class InputBuffer {
    public:
        // Start with rollover so that server can ack tick 0
        uint16_t ack_tick = -1;
        std::queue<MovementInput> buffer;

        void push(MovementInput input) {
            /*
             * Push a copy of the given MovementInput into the buffer
             */
            buffer.push(input);
        }

        void flushUpTo(uint16_t new_ack_tick) {
            /*
             * Inclusively and idempotently flush input buffer up to new_ack_tick
             * new_ack_tick is the most recently acknowledged tick number by the server
             */
            int16_t num_to_flush = (int16_t) (new_ack_tick - ack_tick);
            if (num_to_flush < 0 || num_to_flush > buffer.size()) {
                return;
            }
            for (int i = 0; i < num_to_flush; i++) {
                buffer.pop();
            }
            ack_tick = new_ack_tick;
        }
};
