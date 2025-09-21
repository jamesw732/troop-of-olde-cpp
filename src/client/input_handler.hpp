#pragma once
#include "raylib-cpp.hpp"
#include "flecs.h"
#define ENET_IMPLEMENTATION
#include "enet.h"

#include "shared/components/inputs.hpp"


class InputHandler {
    public:
    MovementInput process_movement_inputs() {
        MovementInput input{0, 0};
        if (IsKeyDown(KEY_W)) {
            input[1]--;
        }
        if (IsKeyDown(KEY_S)) {
            input[1]++;
        }
        if (IsKeyDown(KEY_A)) {
            input[0]--;
        }
        if (IsKeyDown(KEY_D)) {
            input[0]++;
        }
        return input;
    }
};
