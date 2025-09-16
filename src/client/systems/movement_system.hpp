#pragma once
#include "raylib-cpp.hpp"
#include "enet.h"

#include "shared/components/physics.hpp"


void movement_system(ENetPeer* peer, Transformation& t, MovementInput& input);
