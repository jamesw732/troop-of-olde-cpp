#pragma once
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
