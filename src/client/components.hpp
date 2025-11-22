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
    raylib::Vector3 val{};
};

struct PrevSimRotation {
    raylib::Vector3 val{};
};

struct LerpTimer {
    float val = 0;
};

struct AckTick {
    uint16_t val{static_cast<uint16_t>(-1)};
};

struct NetworkMap {
    std::unordered_map<uint32_t, flecs::entity> netid_to_entity;
};

struct CameraInput {
    int8_t rot_x = 0;
    raylib::Vector2 mouse_rot{};
    bool reset = false;
    float scroll = 0;
};

struct CamRotation {
    float x = 0;
    float y = 0;
};

struct CamDistance {
    float val = 10;
};
