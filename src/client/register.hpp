#include "flecs.h"

#include "components.hpp"


inline void register_client_components(flecs::world world) {
    world.component<LerpTimer>();
    world.component<PrevPredPosition>();
    world.component<RenderPosition>();
    world.component<PrevPredRotation>();
    world.component<RenderRotation>();
    world.component<CamRotation>();
    world.component<CamDistance>();
    world.component<LocalPlayer>();
    world.component<AckTick>();
    world.component<Color>();

    world.set<NetworkMap>({});
}
