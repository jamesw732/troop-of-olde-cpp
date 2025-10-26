#include "flecs.h"

#include "../helpers.hpp"
#include "client/components.hpp"
#include "client/movement.hpp"


void test_movement_systems() {
    float dt = 1.0 / 60;
    flecs::world world;
    InputBuffer input_buffer;
    uint16_t movement_tick = 0;
    world.set<NetworkMap>({});

    register_movement_target_system(world);
    register_movement_recv_system(world);
    register_movement_reconcile_system(world, input_buffer);
    register_movement_system(world);
    register_movement_tick_system(world, movement_tick);
    register_movement_lerp_system(world, dt);

    flecs::entity character = world.entity("LocalPlayer");
    character.add<Position>();
    character.add<PrevPosition>();
    character.add<TargetPosition>();
    character.add<LerpTimer>();
    character.add<AckTick>();
    character.add<LocalPlayer>();
    character.add<NetworkId>();
    character.set<NetworkId>({0});
    character.add<MovementInput>();
    character.set<MovementInput>({1, 0});
    character.add<MovementUpdateBatchPacket>();
    auto& netid_to_entity = world.get_mut<NetworkMap>().netid_to_entity;
    netid_to_entity[character.get<NetworkId>()] = character;

    input_buffer.push({1, 0});
    world.progress(1.0 / 60);
    assertClose(0.0, character.get<Position>().val.x);
    world.progress(1.0 / 60);
    world.progress(1.0 / 60);
    input_buffer.push({1, 0});
    assertClose(0.25, character.get<TargetPosition>().val.x);
    world.progress(1.0 / 60);
    assertClose(0.25, character.get<Position>().val.x);
    world.progress(1.0 / 60);
    world.progress(1.0 / 60);
    input_buffer.push({1, 0});
    assertClose(0.5, character.get<TargetPosition>().val.x);
    world.progress(1.0 / 60);
    world.progress(1.0 / 60);

    MovementUpdate update{0, 0, {{10, 0, 0}}};
    MovementUpdateBatchPacket packet{{update}};
    character.set<MovementUpdateBatchPacket>(packet);
    world.progress(1.0 / 60);
    assertClose(10.75, character.get<TargetPosition>().val.x);
}

int main() {
    test_movement_systems();
}
