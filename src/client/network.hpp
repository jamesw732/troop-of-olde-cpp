#pragma once
#include <deque>
#include <memory>
#include <vector>

#include "../shared/util.hpp"

struct NetworkImpl;


class Network {
  private:
      std::unique_ptr<NetworkImpl> impl;
  public:
    std::deque<std::vector<uint8_t>> packets;

    Network();
    ~Network();

    void connect();

    void connect(std::string host_addr, int host_port);

    void queue_data_unreliable(const Buffer& buffer, const size_t size);

    void queue_data_reliable(const Buffer& buffer, const size_t size);

    void send_network_buffer();

    void process_events();

    void disconnect();
};
