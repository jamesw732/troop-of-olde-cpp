#pragma once
#include "raylib-cpp.hpp"

#include "util.hpp"

inline std::string vector3_to_string(raylib::Vector3& v){
    return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
}

inline std::ostream& operator<<(std::ostream& os, const raylib::Vector3& v) {
    os << "Vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}
