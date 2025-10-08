#pragma once
#include "enet.h"

struct Connection {
    ENetPeer* peer;
};

struct NeedsSpawnBatch {};
