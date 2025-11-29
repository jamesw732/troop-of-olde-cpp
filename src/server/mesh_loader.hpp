#pragma once
#include "raylib-cpp.hpp"

struct CollisionMesh {
    float* vertices;
    int triangle_count;
    unsigned short* indices;
};

CollisionMesh LoadCollisionMesh(const char* file_name);
Mesh* LoadMesh(const char* file_name);
