#pragma once
#include "raylib-cpp.hpp"
#include "flecs.h"
#define ENET_IMPLEMENTATION
#include "enet.h"

#include "shared/components/movement.hpp"


class InputHandler {
    public:
    MovementInput process_movement_inputs() {
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
