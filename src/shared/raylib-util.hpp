#pragma once
#include "raylib-cpp.hpp"

#include "util.hpp"


inline std::ostream& operator<<(std::ostream& os, const raylib::Vector3& v) {
    os << "Vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const raylib::Vector2& v) {
    os << "Vec2(" << v.x << ", " << v.y << ")";
    return os;
}
