#include "shared/pch.hpp"

#include "server/components.hpp"
#include "server/disconnect.hpp"
#include "server/network.hpp"
#include "server/mesh_loader.hpp"
#include "server/movement.hpp"
#include "server/login.hpp"
#include "server/packet_handler.hpp"
#include "shared/components.hpp"
#include "shared/const.hpp"
#include "shared/util.hpp"
#include "shared/register.hpp"
#include "shared/serialize.hpp"


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

    Model world_model = LoadServerModel(MODEL_DIR "sample_world.glb");
    /* print_mesh_vertices(*world_model.meshes); */

    loaded_meshes["sample_world"] = world_model;
    auto terrain = world.entity("World");
    terrain.set<Color>(BLUE);
    terrain.set<ModelType>({"mesh"});
    terrain.set<ModelPointer>({&world_model});
    terrain.add<SimPosition>();
    terrain.add<SimRotation>();
    terrain.add<Scale>();
    terrain.add<Terrain>();
    terrain.set<Scale>({{10, 10, 10}});

    register_components(world);

    register_movement_system(world);
    register_movement_networking_system(world, network);
    register_movement_prediction_system(world);

    register_batch_spawn_system(world, network);
    register_spawn_broadcast_system(world, network);
    register_disconnect_system(world, network);


    // Main game loop
    while (true)
    {
        float dt = GetFrameTime();
        network.process_events();
        packet_handler.handle_packets(network.packets);
        world.progress(dt);
        network.send_network_buffer();
    }

    return 0;
}