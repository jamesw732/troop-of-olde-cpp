#pragma once
#include <memory>

#include "../shared/network-components.hpp"
#include "../shared/util.hpp"


struct RecvPacket {
    uint32_t client_id;
    std::vector<uint8_t> data;
};

struct NetworkImpl;

class Network {
  private:
    uint32_t client_id_counter = 0;
    std::unique_ptr<NetworkImpl> impl;
    std::ofstream out_log_file;
    std::ofstream in_log_file;

    void handle_disconnect();

  public:
    std::vector<RecvPacket> packets;

    Network();
    ~Network();

    bool create();

    void process_events();

    void queue_data_unreliable(const uint32_t& client_id, const Buffer& buffer, const size_t size);

    void queue_data_reliable(const uint32_t& client_id, const Buffer& buffer, const size_t size);

    void send_network_buffer();

    uint32_t cast_raw_id(void* raw_id);

    void open_log_files();
    void close_log_files();
};
