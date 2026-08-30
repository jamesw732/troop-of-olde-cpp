#include <chrono>

#include "client/animation.hpp"
#include "client/camera.hpp"
#include "client/components.hpp"
#include "client/disconnect.hpp"
#include "client/fonts.hpp"
#include "client/input.hpp"
#include "client/network.hpp"
#include "client/models.hpp"
#include "client/movement.hpp"
#include "client/packet_handler.hpp"
#include "client/render.hpp"
#include "client/register.hpp"
#include "mapgen/mapgen-util.hpp"
#include "shared/components.hpp"
#include "shared/register.hpp"
#include "shared/serialize.hpp"

/* #ifdef _WIN32 */
/* extern "C" { */
/*     __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001; */
/* } */
/* #endif */


int main()
{
    int const targetFPS = 60;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(targetFPS);
    // Initialize window attributes
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Troop of Olde");

    // Initialize camera attributes
    Camera3D camera;
    camera.position = Vector3{0.0F, 10.0F, 10.0F};
    camera.target = Vector3{0.0F, 0.0F, 0.0F};      // Camera looking at point
    camera.up = Vector3{0.0F, 1.0F, 0.0F};
    camera.fovy = 45.0F;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

    const Font cascadiaMono = LoadFont(cascadiaMonoPath.c_str());

    flecs::world world;
    Network network;
    network.connect();
    const ClientLoginPacket login{.name={"Player"}, .pos={0, 1, 0}, .rot={0, 180, 0}};
    auto [buffer, size] = serialize(login);
    network.queue_data_reliable(buffer, size);
    network.send_network_buffer();

    InputBuffer input_buffer;
    uint16_t movement_tick = 0;
    float dt = 0;

    register_components(world);
    register_client_components(world);

    std::unordered_map<std::string, ModelAsset> loaded_models;
    loaded_models.reserve(128);

    load_all_models(loaded_models);

    /* print_mesh_vertices(*loaded_models["sample_world"].meshes); */

    Map map(2, 1);
    map.grid[0] = {CellType::Normal, Direction::Right};
    map.grid[1] = {CellType::Normal, Direction::Left};
    for (int row = 0; row < map.rows; row++) {
        for (int col = 0; col < map.cols; col++) {
            MapCell cell = map.get({col, row});
            std::string room_name = "room_" + std::to_string((int) cell.direction);
            auto room = world.entity();
            room.set<Color>(GRAY);
            room.set<ModelPointer>({&loaded_models[room_name].model});
            room.add<Scale>();
            room.add<SimPosition>();
            room.add<SimRotation>();
            room.add<RenderPosition>();
            room.add<RenderRotation>();
            room.add<Terrain>();
            room.set<Scale>({{1, 1, 1}});
            Vector3 position{static_cast<float>(col * ROOM_SIZE), 0, static_cast<float>(row * ROOM_SIZE)};
            room.set<SimPosition>({position});
            room.set<RenderPosition>({position});
        }
    }

    auto ManualPhase = world.entity("ManualPhase");

    register_movement_input_aggregate_system(world);
    register_movement_input_buffer_system(world, input_buffer);

    register_movement_recv_system(world);
    register_movement_lerp_reset_system(world);
    register_movement_prediction_reset_system(world);
    register_movement_reconcile_system(world, input_buffer);

    register_camera_input_system(world);

    register_movement_transmit_system(world, network, input_buffer, movement_tick);
    register_movement_tick_system(world, movement_tick);

    register_locomotion_tick_system(world, input_buffer);
    register_animation_recv_system(world);
    register_locomotion_phase_system(world);
    register_locomotion_pose_system(world);
    register_set_render_pose_system(world);
    register_locomotion_blend_system(world);

    register_movement_lerp_system(world);
    register_camera_update_system(world, camera);

    auto render_sys = register_render_system(world, camera, ManualPhase);
    auto anim_render_sys = register_animation_render_system(world, camera, ManualPhase);

    register_movement_input_cleanup_system(world);

    register_disconnect_system(world);

    PacketHandler packet_handler(world, loaded_models);

    // Main game loop
    while (!WindowShouldClose())
    {
        dt = GetFrameTime();
        network.process_events();
        packet_handler.handle_packets(network.packets);
        // Progress all fixed-timestep ECS timers
        world.progress(dt);
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
            anim_render_sys.run();
        EndDrawing();
        // Send all messages to server
        network.send_network_buffer();
    }
    CloseWindow();
    network.disconnect();
    network.close_log_files();

    return 0;
}