#pragma once
#include <array>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <string>
#include <unordered_map>
#include <vector>

#include "bitsery/bitsery.h"
#include "bitsery/adapter/buffer.h"
#include "bitsery/traits/vector.h"
#include "bitsery/traits/string.h"
#include "bitsery/traits/array.h"
#include "raylib.h"
#include "external/cgltf.h"
#include "flecs.h"

#ifdef DEBUG
    #include "dbg.h"
#else
    #define dbg(...) ((void)0)
#endif
