#pragma once
#include "enet.h"

#include "shared/components/physics.hpp"


void movement_networking_system(ENetPeer* peer, MovementInput& input);
