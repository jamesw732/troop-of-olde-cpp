#pragma once
#include <cstdint>
#include <deque>
#include <iostream>

#include "raylib-cpp.hpp"

#include "components.hpp"
#include "../shared/components.hpp"
#include "../shared/raylib-util.hpp"


struct InputHandler {
    MovementInput get_movement_input() {
        MovementInput input;
        if (IsKeyDown(KEY_W)) {
            input.z--;
        }
        if (IsKeyDown(KEY_S)) {
            input.z++;
        }
        if (IsKeyDown(KEY_Q)) {
            input.x--;
        }
        if (IsKeyDown(KEY_E)) {
            input.x++;
        }
        if (IsKeyDown(KEY_A)) {
            input.rot_y++;
        }
        if (IsKeyDown(KEY_D)) {
            input.rot_y--;
        }
        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))  {
            input.mouse_rot_y = raylib::Mouse::GetDelta().x;
        }
        return input;
    }

    CameraInput get_camera_input() {
        CameraInput input;
        if (IsKeyDown(KEY_UP)) {
            input.rot_x++;
        }
        if (IsKeyDown(KEY_DOWN)) {
            input.rot_x--;
        }
        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
            input.mouse_rot.x = raylib::Mouse::GetDelta().y;
        }
        else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            input.mouse_rot.x = raylib::Mouse::GetDelta().y;
            input.mouse_rot.y = raylib::Mouse::GetDelta().x;
        }
        else {
            input.reset = true;
        }
        return input;
    }

    int8_t get_updown_keyboard_rotation() {
        int8_t ret{0};
        if (IsKeyDown(KEY_UP)) {
            ret++;
        }
        if (IsKeyDown(KEY_DOWN)) {
            ret--;
        }
        return ret;
    }
};

class InputBuffer {
  public:
    // Start with rollover so that server can ack tick 0
    uint16_t ack_tick = -1;
    // std::queue<MovementInput> buffer;
    std::deque<MovementInput> buffer;

    void push(const MovementInput& input) {
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

    MovementInput back() {
        return buffer.back();
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

inline std::ostream& operator<<(std::ostream& os, const InputBuffer& buf) {
    os << "Ack tick: " << buf.ack_tick << ",\n" << "Contents: {";
    for (auto it = buf.buffer.begin(); it != buf.buffer.end(); it++) {
        std::cout << *it;
        if (std::next(it) != buf.buffer.end()) {
            std::cout << ", ";
        }
    }
    std::cout << "}\n";
    return os;
}
