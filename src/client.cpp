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


int main()
{
    int const targetFPS = 60;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    // Initialize window attributes
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Troop of Olde");

    // Initialize camera attributes
    raylib::Camera3D camera;
    camera.position = raylib::Vector3(0.0F, 10.0F, 10.0F);
    camera.target = raylib::Vector3(0.0F, 0.0F, 0.0F);      // Camera looking at point
    camera.up = raylib::Vector3(0.0F, 1.0F, 0.0F);
    camera.fovy = 45.0F;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

    const Font cascadiaMono = LoadFont(cascadiaMonoPath.c_str());

    flecs::world world;
    Network network;
    network.connect();
    const ClientLoginPacket login{.name={"Player"}, .pos={0, 1, 0}, .rot={0, 0, 0}};
    auto [buffer, size] = serialize(login);
    network.queue_data_reliable(buffer, size);
    network.send_network_buffer();

    PacketHandler packet_handler(world);

    InputHandler input_handler;
    InputBuffer input_buffer;
    uint16_t movement_tick = 0;
    float dt = 0;

    register_components(world);
    register_client_components(world);

    std::unordered_map<std::string, Model> loaded_models;
    loaded_models.reserve(128);

    loaded_models["sample_world"] = LoadModel((MODEL_DIR "sample_world.glb"));

    auto terrain = world.entity("World");
    terrain.set<Color>(BLUE);
    terrain.set<ModelType>({"mesh"});
    terrain.set<ModelPointer>({&loaded_models["sample_world"]});
    terrain.add<Scale>();
    terrain.add<SimPosition>();
    terrain.add<SimRotation>();
    terrain.add<RenderPosition>();
    terrain.add<RenderRotation>();
    terrain.add<Terrain>();
    terrain.set<Scale>({{10, 10, 10}});
    // TODO: Load terrain from disk
    // auto floor = world.entity("Floor");
    // floor.set<SimPosition>({});
    // floor.set<SimRotation>({});
    // floor.set<RenderPosition>({});
    // floor.set<RenderRotation>({});
    // floor.set<Scale>({{10, 0, 10}});
    // floor.set<Color>(BLUE);
    // floor.set<ModelType>({"3d_quad"});
    // floor.add<Terrain>();

    auto ManualPhase = world.entity("ManualPhase");
    register_movement_recv_system(world);
    register_movement_reconcile_system(world, input_buffer);
    register_movement_input_system(world, input_handler, input_buffer);
    register_movement_system(world, input_buffer);
    register_movement_transmit_system(world, network, input_buffer, movement_tick);
    register_movement_tick_system(world, movement_tick);
    register_movement_lerp_reset_system(world);
    register_movement_lerp_system(world);
    register_camera_input_system(world, input_handler);
    // auto render_terrain_sys = register_terrain_render_system(world, camera, ManualPhase);
    // auto render_sys = register_character_render_system(world, camera, ManualPhase);
    auto render_sys = register_render_system(world, camera, ManualPhase);
    register_disconnect_system(world);

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
            std::string key_indicator = "    ";
            if (IsKeyDown(KEY_W)) key_indicator[0] = 'W';
            if (IsKeyDown(KEY_A)) key_indicator[1] = 'A';
            if (IsKeyDown(KEY_S)) key_indicator[2] = 'S';
            if (IsKeyDown(KEY_D)) key_indicator[3] = 'D';
            DrawTextEx(cascadiaMono, key_indicator.c_str(), {10, 40}, 20, 2, DARKGRAY);
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