#pragma once
#include <iostream>

#include "raylib.h"

#include "util.hpp"

inline std::ostream& operator<<(std::ostream& os, const Matrix& m) {
    os << "Matrix[\n[" << m.m0 << ", " << m.m1 << ", " << m.m2 << ", " << m.m3 << "],\n"
        << "[" << m.m4 << ", " << m.m5 << ", " << m.m8 << ", " << m.m9 << "],\n"
        << "[" << m.m8 << ", " << m.m9 << ", " << m.m10 << ", " << m.m11 << "],\n"
        << "[" << m.m12 << ", " << m.m13 << ", " << m.m14 << ", " << m.m15 << "],\n"
        "]\n";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Vector3& v) {
    os << "Vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Vector2& v) {
    os << "Vec2(" << v.x << ", " << v.y << ")";
    return os;
}


inline void print_mesh_vertices(const Mesh& mesh) {
    std::cout << mesh.vertexCount << "\n";
    std::cout << mesh.triangleCount << "\n";
    float* vertices = mesh.vertices;
    for (int i = 0; i < mesh.vertexCount; i += 3) {
        Vector3 v = Vector3{
            vertices[i],
            vertices[i + 1],
            vertices[i + 2]
        };
        std::cout << v << "\n";
    }
}
