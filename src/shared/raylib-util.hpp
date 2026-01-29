#pragma once
#include <iostream>

#include "raylib.h"

#include "util.hpp"


inline std::ostream& operator<<(std::ostream& os, const Vector3& v) {
    os << "Vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Vector2& v) {
    os << "Vec2(" << v.x << ", " << v.y << ")";
    return os;
}
