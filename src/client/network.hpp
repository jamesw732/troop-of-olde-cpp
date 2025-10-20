#pragma once
#include <deque>
#include <vector>

#include "enet.h"
#include "flecs.h"

#include "shared/util.hpp"

struct NetworkImpl;


class Network {
  private:
    ENetHost* client = {0};
    ENetPeer* peer = {0};
    ENetAddress address = {0};
    ENetEvent event;
    flecs::world world;
  public:
    std::deque<std::vector<uint8_t>> packets;

    Network(flecs::world& world);

    void connect();

    void connect(std::string host_addr, int host_port);

    void queue_data_unreliable(const Buffer& buffer, const size_t size);

    void queue_data_reliable(const Buffer& buffer, const size_t size);

    void send_network_buffer();

    void process_events();

    void disconnect();
};
