#pragma once
#include <cassert>
#include <string>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

#include "bitsery/bitsery.h"
#include "bitsery/adapter/buffer.h"
#include "bitsery/traits/vector.h"
#include "bitsery/traits/string.h"
#include "bitsery/traits/array.h"

#ifdef DEBUG
    #include "dbg.h"
#else
    #define dbg(...) ((void)0)
#endif

using Buffer = std::vector<uint8_t>;
using OutputAdapter = bitsery::OutputBufferAdapter<Buffer>;
using InputAdapter = bitsery::InputBufferAdapter<uint8_t*>;

inline void print_buffer(const std::vector<uint8_t>& buffer) {
    for (uint8_t byte: buffer) {
        std::cout << std::hex
            << std::setw(2)
            << std::setfill('0')
            << static_cast<int>(byte) << ' ';
    }
    std::cout << std::dec << std::endl;
}

inline float angle_slerp(float a0, float a1, float t) {
    assert(a1 - a0 + 540 > 0);
    float diff = fmodf(a1 - a0 + 540, 360) - 180;
    return fmodf(a0 + diff * t, 360);
}
