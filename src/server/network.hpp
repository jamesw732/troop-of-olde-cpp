#pragma once
#include <deque>
#include <iostream>
#include <unordered_map>

#include "enet.h"
#include "flecs.h"

#include "shared/network_components.hpp"
#include "shared/const.hpp"
#include "shared/util.hpp"

struct RecvPacket {
    flecs::entity e;
    std::vector<uint8_t> packet_data;
};

class Network {
  private:
    uint32_t network_id_counter = 0;
    flecs::world world;
    ENetHost* server = {0};
    ENetAddress address = {0};
    ENetEvent event;
    std::unordered_map<NetworkId, ENetPeer*> netid_to_peer;

  public:
    std::deque<RecvPacket> packets;

    Network(flecs::world& w);

    bool create();

    void process_events();

    void queue_data_unreliable(const NetworkId& network_id, const Buffer& buffer, const size_t size);

    void queue_data_reliable(const NetworkId& network_id, const Buffer& buffer, const size_t size);

    void send_network_buffer();

    ecs_entity_t cast_raw_id(void* raw_id);
};
