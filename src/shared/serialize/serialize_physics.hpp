#pragma once
#include "helpers.hpp"
#include "shared/components/physics.hpp"
#include "shared/serialize/serialize_vector3.hpp"

template <typename S>
void serialize(S& s, Position& pos) {
    serialize(s, pos.val);
}
