#pragma once
#include <tuple>

#include "helpers.hpp"
#include "shared/serialize/serialize_movement.hpp"
#include "shared/serialize/serialize_physics.hpp"
#include "shared/serialize/serialize_vector3.hpp"


template<typename T>
inline std::tuple<Buffer, size_t> serialize(T& obj) {
    Buffer buffer;
    bitsery::Serializer<OutputAdapter> ser{OutputAdapter{buffer}};
    ser.object(obj);
    ser.adapter().flush();
    size_t size = ser.adapter().writtenBytesCount();

    return std::tuple<Buffer, size_t>{buffer, size};
}
