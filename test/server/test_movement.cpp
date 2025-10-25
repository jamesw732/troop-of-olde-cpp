#include <iostream>

#include "flecs.h"

#include "server/movement.hpp"
#include "../helpers.hpp"

void test_movement_system() {
    flecs::world world;
    flecs::entity e = world.entity();
    e.add<Position>();
    e.add<ClientMoveTick>();
    e.add<MovementInputPacket>();
    world.progress();
    register_movement_system(world);

    MovementInputPacket packet{1};
    packet.inputs.push_back({1, 0});
    e.set<MovementInputPacket>(packet);
    world.progress(1.0 / 20);
    assertClose(e.get<Position>().val.x, 0.25);
    assertEquals(e.get<ClientMoveTick>().val, 1);
    world.progress(1.0 / 20);

    packet.inputs.push_back({1, 0});
    packet.inputs.push_back({1, 0});
    packet.tick = 3;
    e.set<MovementInputPacket>(packet);
    world.progress(1.0 / 20);
    assertClose(e.get<Position>().val.x, 0.75);
    assertEquals(e.get<ClientMoveTick>().val, 3);
}

int main() {
    test_movement_system();
}
