#pragma once
#include <cstdint>

#include "raylib-cpp.hpp"

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
