#include <cstdint>
#include <iostream>
#include <array>
#include <bitset>
#include <string>

#include "raylib-cpp.hpp"
#include "flecs.h"
#define ENET_IMPLEMENTATION
#include "enet.h"

#include "server/components.hpp"
#include "server/network.hpp"
#include "server/movement.hpp"
#include "server/login.hpp"
#include "shared/components.hpp"
#include "shared/const.hpp"
#include "shared/util.hpp"
#include "shared/register.hpp"
#include "shared/serialize.hpp"


int main(void)
{
    flecs::world world;
    Network network(world);
    if (network.create() > 0) {
        return 1;
    }

    register_components(world);

    register_movement_system(world);
    register_movement_networking_system(world);
    register_batch_spawn_system(world);

    // Main game loop
    while (true)
    {
        float dt = GetFrameTime();
        network.process_events();
        world.progress(dt);
        enet_host_flush(network.server);
    }

    return 0;
}