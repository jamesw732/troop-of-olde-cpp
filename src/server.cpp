#include <cstdint>
#include <iostream>
#include <array>
#include <bitset>
#include <string>

#include "raylib-cpp.hpp"
#include "flecs.h"
#define ENET_IMPLEMENTATION
#include "enet.h"

#include "server/components/networking.hpp"
#include "server/network.hpp"
#include "server/systems/movement_systems.hpp"
#include "server/systems/login_systems.hpp"
#include "shared/components/physics.hpp"
#include "shared/components/movement.hpp"
#include "shared/components/ticks.hpp"
#include "shared/const.hpp"
#include "shared/util.hpp"
#include "shared/serialize/serialize_movement.hpp"
#include "shared/serialize/serialize_physics.hpp"
#include "shared/serialize/helpers.hpp"


int main(void)
{
    flecs::world world;
    Network network(world);
    if (network.create() > 0) {
        return 1;
    }

    register_login_system(world);
    register_movement_system(world);
    register_movement_networking_system(world);

    // Main game loop
    while (true)
    {
        float dt = GetFrameTime();
        network.process_events();
        world.progress(dt);
    }

    return 0;
}