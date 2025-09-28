#include <Functions.hpp>
#include <array>
#include <bitsery/deserializer.h>
#include <cstdint>
#include <iostream>
#include <bitset>

#include "raylib-cpp.hpp"
#include "flecs.h"
#define ENET_IMPLEMENTATION
#include "enet.h"

#include "client/components/player.hpp"
#include "client/fonts.hpp"
#include "client/input_handler.hpp"
#include "client/input_buffer.hpp"
#include "client/network.hpp"
#include "client/systems/movement_systems.hpp"
#include "client/systems/render_systems.hpp"
#include "shared/components/physics.hpp"
#include "shared/const.hpp"
#include "shared/helpers/movement.hpp"
#include "shared/serialize/helpers.hpp"
#include "shared/serialize/serialize_vector3.hpp"
#include "shared/serialize/serialize_physics.hpp"
#include "shared/util.hpp"


int main(void)
{
    // Initialize window attributes
    const int screenWidth = 800;
    const int screenHeight = 450;
    SetTargetFPS(60);
    InitWindow(screenWidth, screenHeight, "Troop of Olde");

    // Initialize camera attributes
    raylib::Camera3D camera;
    camera.position = raylib::Vector3(0.0f, 10.0f, 10.0f);
    camera.target = raylib::Vector3(0.0f, 0.0f, 0.0f);      // Camera looking at point
    camera.up = raylib::Vector3(0.0f, 1.0f, 0.0f);
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

    raylib::Font cascadiaMono = raylib::LoadFont(cascadiaMonoPath);

    flecs::world world;
    Network network(world);
    network.connect();

    // TEMPORARY: Initialize player character
    // TODO: Character should be created by server
    auto player_e = world.entity("LocalPlayer");
    player_e.add<Position>();
    player_e.add<TargetPosition>();
    player_e.add<PrevPosition>();
    player_e.add<LerpTimer>();
    player_e.add<MovementInput>();
    player_e.add<LocalPlayer>();
    player_e.add<ServerMovementUpdate>();

    InputHandler input_handler;
    InputBuffer input_buffer;
    uint16_t movement_tick = 0;
    float dt;

    auto ManualPhase = world.entity("ManualPhase");
    register_movement_target_system(world);
    register_movement_reconcile_system(world, input_buffer);
    register_movement_input_system(world, input_handler, input_buffer);
    register_movement_system(world);
    register_movement_networking_system(world, network.peer, input_buffer, movement_tick);
    register_movement_tick_system(world, movement_tick);
    auto move_lerp_sys = register_movement_lerp_system(world, dt);
    auto render_sys = register_render_system(world, camera, ManualPhase);

    // Main game loop
    while (!WindowShouldClose())
    {
        dt = GetFrameTime();
        network.process_events();
        // Progress all fixed-timestep ECS timers
        world.progress(dt);
        // Do rendering
        BeginDrawing();
            // Draw the UI
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
                DrawGrid(10, 1.0f);
            EndMode3D();
            std::string key_indicator = "    ";
            if (IsKeyDown(KEY_W)) key_indicator[0] = 'W';
            if (IsKeyDown(KEY_A)) key_indicator[1] = 'A';
            if (IsKeyDown(KEY_S)) key_indicator[2] = 'S';
            if (IsKeyDown(KEY_D)) key_indicator[3] = 'D';
            raylib::DrawTextEx(cascadiaMono, key_indicator, {10, 40}, 20, 2, DARKGRAY);
            // cascadiaMono.DrawText(key_indicator, {10, 40}, 20, 2, DARKGRAY);
            DrawFPS(10, 10);
            // Draw each entity in the scene
            render_sys.run();
        EndDrawing();
        // Send all messages to server
        enet_host_flush(network.client);
    }
    CloseWindow();

    return 0;
}