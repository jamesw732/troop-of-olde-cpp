#include <iostream>

#include "flecs.h"

#include "server/movement.hpp"
#include "../helpers.hpp"

void test_movement_system() {
    flecs::world world;
    flecs::entity e = world.entity();
    e.add<SimPosition>();
    e.add<SimRotation>();
    e.add<ClientMoveTick>();
    e.add<MovementInputPacket>();
    world.progress();
    register_movement_system(world);

    MovementInputPacket packet{1};
    packet.inputs.push_back({1, 0, 1});
    e.set<MovementInputPacket>(packet);
    world.progress(1.0 / 20);
    assertClose(0.25, e.get<SimPosition>().val.x);
    assertClose(5.0, e.get<SimRotation>().y);
    assertEquals(1, e.get<ClientMoveTick>().val);
    world.progress(1.0 / 20);

    packet.inputs.push_back({1, 0, 1});
    packet.inputs.push_back({1, 0, 1});
    packet.tick = 3;
    e.set<MovementInputPacket>(packet);
    world.progress(1.0 / 20);
    assertClose(0.75, e.get<SimPosition>().val.x);
    assertClose(15.0, e.get<SimRotation>().y);
    assertEquals(3, e.get<ClientMoveTick>().val);
}

int main() {
    test_movement_system();
}
