#pragma once
#include "raylib-cpp.hpp"

#include "helpers.hpp"

template <typename S>
void serialize (S& s, raylib::Vector3& v) {
    s.value4b(v.x);
    s.value4b(v.y);
    s.value4b(v.z);
}
