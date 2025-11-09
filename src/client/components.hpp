#pragma once
#include <cstdint>
#include <unordered_map>

#include "flecs.h"
#include "raylib-cpp.hpp"

#include "../shared/components.hpp"

struct LocalPlayer {};

struct PrevSimPosition {
    raylib::Vector3 val{};
};

struct RenderPosition {
    raylib::Vector3 val{};
};

struct RenderRotation {
    float val = 0;
};

struct PrevSimRotation {
    float val = 0;
};

struct LerpTimer {
    float val = 0;
};

struct AckTick {
    uint16_t val{static_cast<uint16_t>(-1)};
};

struct NetworkMap {
    std::unordered_map<NetworkId, flecs::entity> netid_to_entity;
};

// Vertical rotation of the head
struct HeadXRotation {
    float val = 30;
};

// Camera's horizontal rotation offset from character's Y rotation
struct HeadYRotation {
    float val = 0;
};

struct CamDistance {
    float val = 10;
};
