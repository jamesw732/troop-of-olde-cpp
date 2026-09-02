#include <atomic>
#include <csignal>

#include "mapgen/mapgen-util.hpp"
#include "mapgen/sparse-prims.hpp"
#include "server/animation.hpp"
#include "server/disconnect.hpp"
#include "server/network.hpp"
#include "server/mesh_loader.hpp"
#include "server/movement.hpp"
#include "server/login.hpp"
#include "server/packet_handler.hpp"
#include "shared/components.hpp"
#include "shared/register.hpp"

std::atomic<bool> running{true};

void handle_signal(int signal) {
    if (signal == SIGINT) {
        running.store(false);
    }
}

int main()
{
    flecs::world world;
    Network network(world);
    if (network.create() > 0) {
        return 1;
    }
    PacketHandler packet_handler{world};

    std::unordered_map<std::string, Model> loaded_models;
    loaded_models.reserve(128);

    for (int i = 1; i < 16; i++) {
        std::string roomname = "room_" + std::to_string(i);
        loaded_models[roomname] = {LoadServerModel((ROOM_DIR + roomname + ".obj").c_str())};
    }
    loaded_models["cube"] = LoadServerModel((MODEL_DIR "cube.glb"));
    loaded_models["humanoid"] = LoadServerModel((MODEL_DIR "humanoid.glb"));
    loaded_models["paladin"] = LoadServerModel((MODEL_DIR "paladin.glb"));
    /* print_mesh_vertices(*world_model.meshes); */

    /* Map map = sparse_prims(8, 8); */
    Map map(2, 1);
    map.grid[0] = {CellType::Normal, Direction::Right};
    map.grid[1] = {CellType::Normal, Direction::Left};
    for (int row = 0; row < map.rows; row++) {
        for (int col = 0; col < map.cols; col++) {
            MapCell cell = map.get({col, row});
            std::string room_name = "room_" + std::to_string((int) cell.direction);
            auto room = world.entity();
            room.set<Color>(GRAY);
            room.set<ModelPointer>({&loaded_models[room_name]});
            room.add<Scale>();
            room.add<SimPosition>();
            room.add<SimRotation>();
            room.add<Terrain>();
            room.set<Scale>({{1, 1, 1}});
            Vector3 position{static_cast<float>(col * ROOM_SIZE), 0, static_cast<float>(row * ROOM_SIZE)};
            room.set<SimPosition>({position});
        }
    }

    register_components(world);

    register_movement_system(world);
    register_animation_tick_system(world);
    register_movement_networking_system(world, network);

    register_batch_spawn_system(world, network, map);
    register_spawn_broadcast_system(world, network);
    register_disconnect_system(world, network);

    std::signal(SIGINT, handle_signal);

    // Main game loop
    while (running.load())
    {
        float dt = GetFrameTime();
        network.process_events();
        packet_handler.handle_packets(network.packets);
        world.progress(dt);
        network.send_network_buffer();
    }
    network.close_log_files();

    return 0;
}