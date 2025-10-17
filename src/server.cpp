#include <cstdint>
#include <iostream>
#include <array>
#include <bitset>
#include <string>

#include "raylib-cpp.hpp"
#include "flecs.h"

#include "server/components.hpp"
#include "server/disconnect.hpp"
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

    register_movement_batch_system(world);
    register_movement_batch_networking_system(world, network);
    register_movement_batch_clear_system(world);

    register_batch_spawn_system(world, network);
    register_spawn_broadcast_system(world, network);
    register_disconnect_system(world);


    // Main game loop
    while (true)
    {
        float dt = GetFrameTime();
        network.process_events();
        world.progress(dt);
        network.send_network_buffer();
    }

    return 0;
}