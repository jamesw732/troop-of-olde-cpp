#pragma once
#include "optional"

#include "raylib.h"

#include "../shared/components.hpp"
#include "../shared/const.hpp"

struct Connected {};
struct NeedsSpawnBatch {};
struct NeedsSpawnBroadcast {};

struct InputBuffer {
    size_t size = 0;
    std::array<MovementInput, MAX_INPUT_BUFFER> inputs{};

    bool empty() {
        return size == 0;
    }

    MovementInput& back() {
        assert(!empty() && size < MAX_INPUT_BUFFER);
        return inputs[size - 1];

    }
};

struct RecvMoveTick {
    uint16_t val = 0;
};

struct CurMoveTick {
    uint16_t val = 0;
};

