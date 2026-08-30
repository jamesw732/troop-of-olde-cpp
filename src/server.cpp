#include <atomic>
#include <csignal>

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

    std::unordered_map<std::string, Model> loaded_meshes;
    loaded_meshes.reserve(128);

    for (int i = 1; i < 16; i++) {
        std::string roomname = "room_" + std::to_string(i);
        loaded_meshes[roomname] = {LoadServerModel((ROOM_DIR + roomname + ".obj").c_str())};
    }
    loaded_meshes["cube"] = LoadServerModel((MODEL_DIR "cube.glb"));
    loaded_meshes["humanoid"] = LoadServerModel((MODEL_DIR "humanoid.glb"));
    loaded_meshes["paladin"] = LoadServerModel((MODEL_DIR "paladin.glb"));
    /* print_mesh_vertices(*world_model.meshes); */

    auto terrain = world.entity("World");
    terrain.set<Color>(BLUE);
    terrain.set<ModelPointer>({&loaded_meshes["room_1"]});
    terrain.add<SimPosition>();
    terrain.add<SimRotation>();
    terrain.add<Scale>();
    terrain.add<Terrain>();
    terrain.set<Scale>({{1, 1, 1}});

    register_components(world);

    register_movement_system(world);
    register_animation_tick_system(world);
    register_movement_networking_system(world, network);

    register_batch_spawn_system(world, network);
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