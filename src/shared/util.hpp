#pragma once
#include <string>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <vector>

#include "raylib-cpp.hpp"


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
