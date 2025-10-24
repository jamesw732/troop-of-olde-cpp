#include "flecs.h"

#include "components.hpp"


inline void register_client_components(flecs::world world) {
    world.component<LerpTimer>();
    world.component<PrevPosition>();
    world.component<TargetPosition>();
    world.component<LocalPlayer>();
    world.component<AckTick>();

    world.set<NetworkMap>({});
}
