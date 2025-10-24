#pragma once
#include <cstdint>
#include <unordered_map>

#include "flecs.h"
#include "raylib-cpp.hpp"

#include "../shared/components.hpp"

struct LocalPlayer {};

struct PrevPosition {
    raylib::Vector3 val{0, 0, 0};
};

struct TargetPosition {
    raylib::Vector3 val{0, 0, 0};
};

struct LerpTimer {
    float val{0};
};

struct AckTick {
    uint16_t val{static_cast<uint16_t>(-1)};
};

struct NetworkMap {
    std::unordered_map<NetworkId, flecs::entity> netid_to_entity;
};
