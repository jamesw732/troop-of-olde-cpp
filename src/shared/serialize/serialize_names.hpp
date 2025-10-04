#pragma once

#include "shared/components/names.hpp"

template<typename S>
void serialize(S& s, DisplayName name) {
    s.text1b(name.name, 32);
}
