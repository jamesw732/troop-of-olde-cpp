#pragma once
#include <cstdint>
#include <deque>
#include <iostream>

#include "shared/components/movement.hpp"


class InputBuffer {
    public:
        // Start with rollover so that server can ack tick 0
        uint16_t ack_tick = -1;
        // Smallest tick in buffer should always be ack tick + 1
        uint16_t buffer_min_tick = 0;
        // std::queue<MovementInput> buffer;
        std::deque<MovementInput> buffer;

        void push(MovementInput input) {
            /*
             * Push a copy of the given MovementInput into the buffer
             */
            buffer.push_back(input);
        }

        MovementInput pop() {
            MovementInput elem = buffer.front();
            buffer.pop_front();
            return elem;
        }

        const bool empty() {
            return buffer.empty();
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
                buffer.pop_front();
            }
            buffer_min_tick = new_ack_tick + 1;
            ack_tick = new_ack_tick;
        }
};
