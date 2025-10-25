#pragma once
#include <cstdint>
#include <deque>
#include <iostream>

#include "../shared/components.hpp"


class InputHandler {
    public:
    MovementInput get_movement_input() {
        MovementInput input{0, 0};
        if (IsKeyDown(KEY_W)) {
            input.z--;
        }
        if (IsKeyDown(KEY_S)) {
            input.z++;
        }
        if (IsKeyDown(KEY_A)) {
            input.x--;
        }
        if (IsKeyDown(KEY_D)) {
            input.x++;
        }
        return input;
    }
};

class InputBuffer {
    public:
        // Start with rollover so that server can ack tick 0
        uint16_t ack_tick = -1;
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
            ack_tick = new_ack_tick;
        }
};
