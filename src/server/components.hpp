#pragma once
#include "../shared/components.hpp"

struct Connected {};
struct NeedsSpawnBatch {};
struct NeedsSpawnBroadcast {};

struct PredPosition {
    raylib::Vector3 val;
};

struct PredRotation {
    raylib::Vector3 val;
};

struct PredGravity {
    float val = 0;;
};

struct PredGrounded {
    bool val = false;;
};
