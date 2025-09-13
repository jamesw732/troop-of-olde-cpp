#include <array>
#include <cstdint>
#include <iostream>
#include <bitset>

#include "raylib-cpp.hpp"
#include "flecs.h"
#define ENET_IMPLEMENTATION
#include "enet.h"

const float MOVE_UPDATE_RATE = 1.0 / 20.0;

typedef std::array<std::int8_t, 2> KeyboardMovement; // x z
typedef raylib::Vector3 Velocity;

struct Transformation {
    raylib::Vector3 pos;
    raylib::Vector3 rot;
    raylib::Vector3 scale;
};

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
        KeyboardMovement km{0, 0};
        if (IsKeyDown(KEY_W)) {
            km[1]--;
        }
        if (IsKeyDown(KEY_S)) {
            km[1]++;
        }
        if (IsKeyDown(KEY_A)) {
            km[0]--;
        }
        if (IsKeyDown(KEY_D)) {
            km[0]++;
        }
        character.set<KeyboardMovement>(km);
    }
};

std::string vector3_to_string(raylib::Vector3& v) {
    return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
}

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

    auto move_sys = world.system<Transformation, KeyboardMovement>()
        .interval(MOVE_UPDATE_RATE)
        .each([&peer](flecs::iter& it, size_t, Transformation& t, KeyboardMovement& km) {
                // CHANGE
                raylib::Vector3 velocity((float) km[0], 0, (float) km[1]);
                velocity = velocity.Normalize();
                velocity = velocity * 0.25;

                t.pos += velocity;

                ENetPacket* packet = enet_packet_create(km.data(), km.size() * sizeof(std::int8_t), 0);
                enet_peer_send(peer, 0, packet);
            }
        );
    auto render_sys = world.system<Transformation>()
        .each([&camera](flecs::iter& it, size_t, Transformation& t) {
                BeginMode3D(camera);
                    DrawCube(t.pos, 2.0f, 2.0f, 2.0f, RED);
                    DrawCubeWires(t.pos, 2.0f, 2.0f, 2.0f, MAROON);
                EndMode3D();
            }
        );

    // TEMPORARY: Initialize player character
    // TODO: Character should be created by server
    Transformation transformComp = {
         {0.0f, 0.0f, 0.0f},
         {0.0f, 0.0f, 0.0f},
         {1.0f, 1.0f, 1.0f}
    };

    auto e = world.entity("character");
    e.add<Transformation>();
    e.set<Transformation>(transformComp);
    e.add<Velocity>();
    e.set<Velocity>({0.0f, 0.0f, 0.0f});
    e.add<KeyboardMovement>();
    e.set<KeyboardMovement>({0, 0});

    // Initialize input handler
    InputHandler input_handler(client, peer, world, e);

    // Main game loop
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        while (enet_host_service(client, &event, 0) > 0) {

        }
        // Handle inputs
        input_handler.process_movement_inputs();
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