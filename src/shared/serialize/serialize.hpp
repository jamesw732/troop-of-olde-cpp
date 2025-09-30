#pragma once
#include "helpers.hpp"
#include "shared/serialize/serialize_movement.hpp"
#include "shared/serialize/serialize_physics.hpp"
#include "shared/serialize/serialize_vector3.hpp"

template<typename T>
inline size_t serialize(T& obj, Buffer& buffer) {
    // Serialize into buffer, return size
    return bitsery::quickSerialization(OutputAdapter{buffer}, obj);
}
