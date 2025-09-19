#pragma once
#include <array>
#include <cstdint>

#include "raylib-cpp.hpp"

typedef std::array<std::int8_t, 2> MovementInput;
typedef raylib::Vector3 Velocity;

struct Transformation {
    raylib::Vector3 pos;
    raylib::Vector3 rot;
    raylib::Vector3 scale;
};
