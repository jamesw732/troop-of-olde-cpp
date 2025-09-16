#include "raylib-cpp.hpp"
#include "flecs.h"
#include "enet.h"

#include "movement_system.hpp"
#include "shared/components/physics.hpp"

void movement_system(Transformation& t, MovementInput& input) {
    raylib::Vector3 velocity((float) input[0], 0, (float) input[1]);
    velocity = velocity.Normalize();
    velocity = velocity * 0.25;

    t.pos += velocity;
}

