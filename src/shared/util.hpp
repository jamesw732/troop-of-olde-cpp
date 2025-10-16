#pragma once
#include <string>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <vector>

#include "raylib-cpp.hpp"
#ifdef DEBUG
    #include "dbg.h"
#else
    #define dbg(...) ((void)0)
#endif

#include "components.hpp"


inline std::string vector3_to_string(raylib::Vector3& v){
    return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
}

inline void print_buffer(const std::vector<uint8_t>& buffer) {
    for (uint8_t byte: buffer) {
        std::cout << std::hex
            << std::setw(2)
            << std::setfill('0')
            << static_cast<int>(byte) << ' ';
    }
    std::cout << std::dec << std::endl;
}

inline std::ostream& operator<<(std::ostream& os, const raylib::Vector3& v) {
    os << "Vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}
