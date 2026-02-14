#include "../shared/pch-networking.hpp"

#include "network.hpp"


struct NetworkImpl {
    ENetHost* client{0};
    ENetPeer* peer{0};
    ENetAddress address{0};
    ENetEvent event;
};


Network::Network() : impl(std::make_unique<NetworkImpl>()) {
    open_log_files();
};
Network::~Network() = default;

void Network::connect() {
    connect("127.0.0.1", 7777);
}

void Network::connect(std::string host_addr, int host_port) {
    if (enet_initialize() != 0) {
        std::cout << "An error occurred while initializing ENET." << std::endl;
        exit(EXIT_FAILURE);
    }
    impl->client = enet_host_create(
        NULL, // Create a client host
        1, // Only allow 1 outgoing connection
        2, // Allow up to 2 channels to be used, 0 and 1
        0, // Assume any amount of incoming bandwidth
        0 // Assume any amount of outgoing bandwidth
    );
    if (impl->client == NULL) {
        std::cout << "An error occurred while trying to create an ENet client host." << std::endl;
        exit(EXIT_FAILURE);
    }
    // Bind server address
    enet_address_set_host(&impl->address,host_addr.c_str());
    impl->address.port = host_port;
    // Initiate the connection, allocating the two channels 0 and 1.
    impl->peer = enet_host_connect(impl->client, &impl->address, 2, 0);
    if (impl->peer == NULL) {
        std::cout << "No available peers for initiating an ENet connection." << std::endl;
        exit(EXIT_FAILURE);
    }
    if (enet_host_service(impl->client, &impl->event, 10000) > 0 && impl->event.type == ENET_EVENT_TYPE_CONNECT) {
        std::cout << "Connection to Troop of Olde server succeeded." << std::endl;
    } else {
        enet_peer_reset(impl->peer);
        std::cout << "Connection to Troop of Olde server failed." << std::endl;
        exit(1);
    }
}

void Network::queue_data_unreliable(const Buffer& buffer, const size_t size){
    ENetPacket* packet = enet_packet_create(buffer.data(), size, 0);
    log(out_log_file, buffer, size);
    enet_peer_send(impl->peer, 0, packet);
}

void Network::queue_data_reliable(const Buffer& buffer, const size_t size){
    ENetPacket* packet = enet_packet_create(buffer.data(), size, ENET_PACKET_FLAG_RELIABLE);
    log(out_log_file, buffer, size);
    enet_peer_send(impl->peer, 1, packet);
}

void Network::send_network_buffer() {
    enet_host_flush(impl->client);
}

void Network::process_events() {
    while (enet_host_service(impl->client, &impl->event, 0) > 0) {
        switch (impl->event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                // Don't do anything here, do logic in connect function
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE: {
                uint8_t* buffer = impl->event.packet->data;
                size_t size = impl->event.packet->dataLength;
                Buffer packet_data(buffer, buffer + size);
                // To be handled by PacketHandler
                packets.push_back(packet_data);
                log(in_log_file, packet_data, size);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT: {
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
                break;
            }
            case ENET_EVENT_TYPE_NONE: {
                break;
            }
        }
    }
}

void Network::disconnect() {
    enet_peer_disconnect(impl->peer, 0);
    while (enet_host_service(impl->client, &impl->event, 0) > 0) {
        switch (impl->event.type) {
            case ENET_EVENT_TYPE_DISCONNECT: {
                return;
            }
            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
                return;
            }
            default: {
                break;
            }
        }
    }
}

void Network::open_log_files() {
    out_log_file.open("client-out.bin", std::ios_base::binary);
    in_log_file.open("client-in.bin", std::ios_base::binary);
    // Replace above with these if you want to append rather than replace
    /* out_log_file.open("client-out.bin", std::ios_base::binary | std::ios_base::app); */
    /* in_log_file.open("client-in.bin", std::ios_base::binary | std::ios_base::app); */
}

void Network::close_log_files() {
    in_log_file.close();
    out_log_file.close();
}
