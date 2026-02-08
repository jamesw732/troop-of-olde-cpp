#pragma once
#include <deque>
#include <iostream>
#include <memory>
#include <unordered_map>

#include "flecs.h"

#include "../shared/network_components.hpp"
#include "../shared/const.hpp"
#include "../shared/util.hpp"


struct RecvPacket {
    flecs::entity e;
    std::vector<uint8_t> packet_data;
};


struct NetworkImpl;

class Network {
  private:
    uint32_t network_id_counter = 0;
    flecs::world world;
    std::unique_ptr<NetworkImpl> impl;
    std::ofstream out_log_file;
    std::ofstream in_log_file;

  public:
    std::deque<RecvPacket> packets;

    Network(flecs::world& w);
    ~Network();

    bool create();

    void process_events();

    void queue_data_unreliable(const NetworkId& network_id, const Buffer& buffer, const size_t size);

    void queue_data_reliable(const NetworkId& network_id, const Buffer& buffer, const size_t size);

    void send_network_buffer();

    ecs_entity_t cast_raw_id(void* raw_id);

    void open_log_files();
    void close_log_files();
};
