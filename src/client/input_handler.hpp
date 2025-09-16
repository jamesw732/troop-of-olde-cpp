#pragma once
#include "raylib-cpp.hpp"
#include "flecs.h"
#define ENET_IMPLEMENTATION
#include "enet.h"

#include "shared/components/physics.hpp"


class InputHandler {
    // Should this just be a system that operates on only the player character?
    public:
    ENetHost* client;
    ENetPeer* peer;
    flecs::world& world;
    flecs::entity& character;

    InputHandler(ENetHost* _client, ENetPeer* _peer, flecs::world& _world, flecs::entity& _character)
        : client(_client), peer(_peer), world(_world), character(_character) {}

    void process_movement_inputs() {
        MovementInput input{0, 0};
        if (IsKeyDown(KEY_W)) {
            input[1]--;
        }
        if (IsKeyDown(KEY_S)) {
            input[1]++;
        }
        if (IsKeyDown(KEY_A)) {
            input[0]--;
        }
        if (IsKeyDown(KEY_D)) {
            input[0]++;
        }
        character.set<MovementInput>(input);
    }
};
