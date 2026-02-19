#pragma once
#include "raylib.h"

#include "../shared/components.hpp"

struct Connected {};
struct NeedsSpawnBatch {};
struct NeedsSpawnBroadcast {};

struct PredPosition {
    Vector3 val{};
};

struct PredRotation {
    Vector3 val{};
};

struct PredGravity {
    float val = 0;;
};

struct PredGrounded {
    bool val = false;;
};
