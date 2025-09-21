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
#include "client/input_handler.hpp"
#include "client/input_buffer.hpp"
#include "client/systems/movement_networking_system.hpp"
#include "client/systems/movement_system.hpp"
#include "client/systems/render_system.hpp"
#include "shared/components/physics.hpp"
#include "shared/const.hpp"
#include "shared/util.hpp"
#include "shared/serialize/helpers.hpp"
#include "shared/serialize/serialize_vector3.hpp"


int main(void)
{
    // Initialize window attributes
    const int screenWidth = 800;
    const int screenHeight = 450;
    SetTargetFPS(60);
    InitWindow(screenWidth, screenHeight, "Troop of Olde");

    // Initialize camera attributes
    Camera3D camera = { 0 };
    camera.position = raylib::Vector3(0.0f, 10.0f, 10.0f);
    camera.target = raylib::Vector3(0.0f, 0.0f, 0.0f);      // Camera looking at point
    camera.up = raylib::Vector3(0.0f, 1.0f, 0.0f);
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

    // Initialize ENet client
    if (enet_initialize() != 0) {
        std::cout << "An error occurred while initializing ENet" << std::endl;
        return EXIT_FAILURE;
    }

    ENetHost* client = {0};
    client = enet_host_create(
        NULL, // Create a client host
        1, // Only allow 1 outgoing connection
        2, // Allow up to 2 channels to be used, 0 and 1
        0, // Assume any amount of incoming bandwidth
        0 // Assume any amount of outgoing bandwidth
    );
    if (client == NULL) {
        std::cout << "An error occurred while trying to create an ENet client host." << std::endl;
        exit(EXIT_FAILURE);
    }
    ENetAddress address = {0};
    ENetEvent event;
    ENetPeer* peer = {0};
    // Bind server address
    enet_address_set_host(&address, "127.0.0.1");
    address.port = 7777;
    // Initiate the connection, allocating the two channels 0 and 1.
    peer = enet_host_connect(client, &address, 2, 0);
    if (peer == NULL) {
        std::cout << "No available peers for initiating an ENet connection." << std::endl;
        exit(EXIT_FAILURE);
    }
    if (enet_host_service(client, &event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT) {
        std::cout << "Connection to Troop of Olde server succeeded." << std::endl;
    } else {
        enet_peer_reset(peer);
        std::cout << "Connection to Troop of Olde server failed." << std::endl;
    }

    // Initialize ECS world and systems
    flecs::world world;

    // TEMPORARY: Initialize player character
    // TODO: Character should be created by server
    Transformation transformComp = {
         {0.0f, 0.0f, 0.0f},
         {0.0f, 0.0f, 0.0f},
         {1.0f, 1.0f, 1.0f}
    };

    auto player_e = world.entity("character");
    player_e.add<Transformation>();
    player_e.set<Transformation>(transformComp);
    player_e.add<Velocity>();
    player_e.set<Velocity>({0.0f, 0.0f, 0.0f});
    player_e.add<MovementInput>();
    player_e.set<MovementInput>({0, 0});
    player_e.add<LocalPlayer>();

    // Initialize input handler
    InputHandler input_handler;
    // Initialize input buffer
    InputBuffer input_buffer;

    // Initialize movement systems
    uint16_t movement_tick = 0;
    auto move_tick_sys = world.system()
        .interval(MOVE_UPDATE_RATE)
        .each([&movement_tick]() {
                movement_tick++;
            }
        );
    auto move_input_sys = world.system<MovementInput, LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([&input_handler, &input_buffer](MovementInput& input, LocalPlayer) {
            input = input_handler.process_movement_inputs();
            input_buffer.push(input);
            }
        );
    auto move_sys = world.system<Transformation, MovementInput, LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([](Transformation& t, MovementInput& input, LocalPlayer) {
                movement_system(t, input);
            }
        );
    auto move_networking_sys = world.system<MovementInput, LocalPlayer>()
        .interval(MOVE_UPDATE_RATE)
        .each([&peer](MovementInput& input, LocalPlayer) {
                movement_networking_system(peer, input);
            }
        );
    // Initialize render system
    auto render_sys = world.system<Transformation>()
        .each([&camera](Transformation& t) {
                render_system(camera, t);
            }
        );

    // Main game loop
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        while (enet_host_service(client, &event, 0) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    break;
                }
                case ENET_EVENT_TYPE_RECEIVE: {
                    raylib::Vector3 pos;
                    Buffer buffer;
                    auto state = bitsery::quickDeserialization(
                        InputAdapter{
                            event.packet->data,
                            event.packet->dataLength
                        },
                        pos
                    );
                    std::cout << "Received position "
                        << vector3_to_string(pos)
                        << " from server."
                        << std::endl;
                }
                case ENET_EVENT_TYPE_DISCONNECT: {
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
                    break;
                }
                case ENET_EVENT_TYPE_NONE: {
                    break;
                }
            }
        }
        // Progress all ECS timers
        world.progress(dt);
        // Do rendering
        BeginDrawing();
            // Draw the UI
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
                DrawGrid(10, 1.0f);
            EndMode3D();
            DrawText("Welcome to the third dimension!", 10, 40, 20, DARKGRAY);
            DrawFPS(10, 10);
            // Draw each entity in the scene
            render_sys.run();
        EndDrawing();
        // Send all messages to server
        enet_host_flush(client);
    }
    CloseWindow();

    return 0;
}