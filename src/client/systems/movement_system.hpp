#pragma once
#include "raylib-cpp.hpp"
#include "enet.h"

#include "shared/components/physics.hpp"
#include "shared/components/movement.hpp"
#include "shared/helpers/movement.hpp"



inline void movement_system(Position& pos, MovementInput& input) {
    process_movement_input(pos, input);
}
