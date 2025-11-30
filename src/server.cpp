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


int main(void)
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
    loaded_meshes["sample_world"] = world_model;

    auto terrain = world.entity("World");
    terrain.set<Color>(BLUE);
    terrain.set<ModelType>({"mesh"});
    terrain.set<ModelPointer>({&world_model});
    terrain.add<SimPosition>();
    terrain.add<SimRotation>();
    terrain.add<Scale>();
    terrain.add<Terrain>();
    // TODO: Load terrain from disk
    // auto floor = world.entity("Floor");
    // floor.set<SimPosition>({});
    // floor.set<SimRotation>({});
    // floor.set<Scale>({{10, 0, 10}});
    // floor.set<Color>(BLUE);
    // floor.set<ModelType>({"3d_quad"});
    // floor.add<Terrain>();

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