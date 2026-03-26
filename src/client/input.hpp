#pragma once
#include <cstdint>
#include <iostream>

#include "raylib.h"

#include "components.hpp"
#include "../shared/components.hpp"
#include "../shared/const.hpp"


/*
 * Get per-movement-tick inputs which impact the character's position, rotation, or velocity
 * NOTE: This does not sample MovementInput.mouse_y_rot, which is a per-frame metric
 */
inline MovementInput get_movement_input() {
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
    input.jump = IsKeyDown(KEY_SPACE);
    return input;
}

// Inputs made which impact the camera's (but not character's!) position or rotation
inline CameraInput get_camera_input() {
    CameraInput input;
    if (IsKeyDown(KEY_UP)) {
        input.rot_x++;
    }
    if (IsKeyDown(KEY_DOWN)) {
        input.rot_x--;
    }
    if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
        input.mouse_rot.x = GetMouseDelta().y;
    }
    else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        input.mouse_rot.x = GetMouseDelta().y;
        input.mouse_rot.y = GetMouseDelta().x;
    }
    else {
        input.reset = true;
    }
    input.scroll = GetMouseWheelMove();
    return input;
}

struct InputBuffer {
    // Start with rollover to be consistent with server acking tick 0 to start
    uint16_t ack_tick = -1;
    uint16_t start_idx = 0;
    uint16_t size = 0;
    bool full = false;
    std::array<MovementInput, MAX_INPUT_BUFFER> buffer;

    const bool empty() {
        return size == 0;
    }

    MovementInput& get_at(size_t i) {
        assert(i < size && !empty());
        return buffer[(start_idx + i) % MAX_INPUT_BUFFER];
    }

    void set_at(const MovementInput& input, size_t i) {
        /* assert(i < size && !empty()); */
        buffer[(start_idx + i) % MAX_INPUT_BUFFER] = input;
    }

    void push(const MovementInput& input) {
        set_at(input, size);
        if (size >= MAX_INPUT_BUFFER) {
            start_idx += size - MAX_INPUT_BUFFER + 1;
            start_idx %= MAX_INPUT_BUFFER;
            return;
        }
        size++;
    }

    MovementInput& pop() {
        assert(!empty());
        MovementInput& input = get_at(0);

        start_idx++;
        start_idx %= MAX_INPUT_BUFFER;
        size--;
        return input;
    }

    MovementInput& back() {
        assert(!empty());
        return get_at(size - 1);
    }

    /*
     * Inclusively and idempotently flush input buffer up to new_ack_tick
     * new_ack_tick is the most recently acknowledged tick number by the server
     */
    void flushUpTo(uint16_t new_ack_tick) {
        int16_t num_to_flush = (int16_t) (new_ack_tick - ack_tick);
        if (num_to_flush <= 0 || num_to_flush > size) {
            return;
        }
        for (int i = 0; i < num_to_flush; i++) {
            pop();
        }
        ack_tick = new_ack_tick;
    }

    void copy_to_vector(std::vector<MovementInput>& vec) {
        for (int i = 0; i < size; i++) {
            vec.push_back(get_at(i));
        }
    }
    void copy_to_array(std::array<MovementInput, MAX_INPUT_BUFFER>& array) {
        for (int i = 0; i < size; i++) {
            array[i] = get_at(i);
        }
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
