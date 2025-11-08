#include "shared/pch.hpp"

#include "client/camera.hpp"
#include "client/components.hpp"
#include "client/disconnect.hpp"
#include "client/fonts.hpp"
#include "client/input.hpp"
#include "client/network.hpp"
#include "client/movement.hpp"
#include "client/packet_handler.hpp"
#include "client/render.hpp"
#include "client/register.hpp"
#include "shared/components.hpp"
#include "shared/const.hpp"
#include "shared/movement.hpp"
#include "shared/register.hpp"
#include "shared/serialize.hpp"
#include "shared/util.hpp"


int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    // Initialize window attributes
    const int screenWidth = 800;
    const int screenHeight = 450;
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
    Network network;
    network.connect();
    ClientLoginPacket login{{"Player"}};
    auto [buffer, size] = serialize(login);
    network.queue_data_reliable(buffer, size);

    PacketHandler packet_handler(world);

    // TEMPORARY: Initialize player character
    // TODO: Character should be created by server

    InputHandler input_handler;
    InputBuffer input_buffer;
    uint16_t movement_tick = 0;
    float dt;

    register_components(world);
    register_client_components(world);

    auto ManualPhase = world.entity("ManualPhase");
    register_movement_target_system(world);
    register_movement_recv_system(world);
    register_movement_reconcile_system(world, input_buffer);
    register_movement_input_system(world, input_handler, input_buffer);
    register_movement_system(world);
    register_movement_networking_system(world, network, input_buffer, movement_tick);
    register_movement_tick_system(world, movement_tick);
    register_disconnect_system(world);
    register_movement_lerp_system(world);
    auto render_sys = register_render_system(world, camera, ManualPhase);

    // Main game loop
    while (!WindowShouldClose())
    {
        dt = GetFrameTime();
        network.process_events();
        packet_handler.handle_packets(network.packets);
        // Progress all fixed-timestep ECS timers
        world.progress(dt);
        update_camera(world, camera);
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
        network.send_network_buffer();
    }
    CloseWindow();
    network.disconnect();

    return 0;
}