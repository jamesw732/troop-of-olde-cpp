#include <iostream>

#include "flecs.h"

#include "server/movement.hpp"
#include "../helpers.hpp"

void test_movement_system() {
    flecs::world world;
    flecs::entity e = world.entity();
    e.add<Position>();
    e.add<Rotation>();
    e.add<ClientMoveTick>();
    e.add<MovementInputPacket>();
    world.progress();
    register_movement_system(world);

    MovementInputPacket packet{1};
    packet.inputs.push_back({1, 0, 1});
    e.set<MovementInputPacket>(packet);
    world.progress(1.0 / 20);
    assertClose(0.25, e.get<Position>().val.x);
    assertClose(5.0, e.get<Rotation>().val);
    assertEquals(1, e.get<ClientMoveTick>().val);
    world.progress(1.0 / 20);

    packet.inputs.push_back({1, 0, 1});
    packet.inputs.push_back({1, 0, 1});
    packet.tick = 3;
    e.set<MovementInputPacket>(packet);
    world.progress(1.0 / 20);
    assertClose(0.75, e.get<Position>().val.x);
    assertClose(15.0, e.get<Rotation>().val);
    assertEquals(3, e.get<ClientMoveTick>().val);
}

int main() {
    test_movement_system();
}
