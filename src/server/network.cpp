#include "../shared/pch-networking.hpp"

#include "network.hpp"
#include "../shared/network-components.hpp"
#include "../shared/const.hpp"
#include "../shared/packet_types.hpp"
#include "../shared/util.hpp"


struct NetworkImpl {
    ENetHost* server = {0};
    ENetAddress address = {0};
    ENetEvent event;
    std::unordered_map<uint32_t, ENetPeer*> client_id_to_peer;
};

Network::Network(): impl(std::make_unique<NetworkImpl>()) {
    open_log_files();
};
Network::~Network() = default;

bool Network::create() {
    // Create ENet server
    if (enet_initialize() != 0) {
        std::cout << "An error occurred while initializing ENET." << std::endl;
        return 1;
    }
    impl->address.host = ENET_HOST_ANY;
    impl->address.port = 7777;
    impl->server = enet_host_create(&impl->address, MAX_CLIENTS, 2, 0, 0);

    if (impl->server == NULL) {
        std::cout << "An error occurred while trying to create an ENet server host." << std::endl;
        return 1;
    }
    std::cout << "Started a server..." << std::endl;
    ENetEvent event;
    return 0;
}

void Network::process_events() {
    // Process networking events
    while (enet_host_service(impl->server, &impl->event, 0) > 0) {
        switch (impl->event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                std::cout << "A new client connected from "
                    // <<  (int) impl->event.peer->address.host
                    << ":"
                    << (int) impl->event.peer->address.port
                    << "." << std::endl;
                impl->client_id_to_peer[client_id_counter] = impl->event.peer;
                impl->event.peer->data = (void*) (uintptr_t) client_id_counter;
                client_id_counter++;
                break;
            }

            case ENET_EVENT_TYPE_RECEIVE: {
                uint32_t client_id = cast_raw_id(impl->event.peer->data);
                uint8_t* buffer = impl->event.packet->data;
                size_t size = impl->event.packet->dataLength;
                Buffer packet_data(buffer, buffer + size);
                // To be handled by PacketHandler
                packets.push_back({client_id, packet_data});
                log(in_log_file, packet_data, size);
                break;
            }

            case ENET_EVENT_TYPE_DISCONNECT: {
                handle_disconnect();
                break;
            }

            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
                handle_disconnect();
                break;
            }

            case ENET_EVENT_TYPE_NONE:
                break;
        }
    }
}

void Network::handle_disconnect() {
    uint32_t client_id = cast_raw_id(impl->event.peer->data);
    std::cout << "Client with Entity id "
        << client_id
        << " disconnected due to timeout."
        << std::endl;
    impl->event.peer->data = NULL;
    impl->client_id_to_peer.erase(client_id);
    enet_peer_reset(impl->event.peer);
    // forward the disconnect to packet_handler as a buffer with just DisconnectPacket enum
    std::vector<uint8_t> buffer;
    buffer.push_back(static_cast<uint8_t>(PacketType::DisconnectPacket));
    packets.push_back({client_id, buffer});
}

void Network::queue_data_unreliable(const uint32_t& client_id, const Buffer& buffer, const size_t size){
    if (!impl->client_id_to_peer.contains(client_id)) return;
    ENetPeer* peer = impl->client_id_to_peer[client_id];
    ENetPacket* packet = enet_packet_create(buffer.data(), size, 0);
    enet_peer_send(peer, 0, packet);
    log(out_log_file, buffer, size);
}

void Network::queue_data_reliable(const uint32_t& client_id, const Buffer& buffer, const size_t size){
    if (!impl->client_id_to_peer.contains(client_id)) return;
    ENetPeer* peer = impl->client_id_to_peer[client_id];
    ENetPacket* packet = enet_packet_create(buffer.data(), size, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 1, packet);
    log(out_log_file, buffer, size);
}

void Network::send_network_buffer() {
    enet_host_flush(impl->server);
}

void Network::open_log_files() {
    out_log_file.open("server-out.bin", std::ios_base::binary);
    in_log_file.open("server-in.bin", std::ios_base::binary);
    // Replace above with these if you want to append rather than replace
    /* out_log_file.open("server-out.bin", std::ios_base::binary | std::ios_base::app); */
    /* in_log_file.open("server-in.bin", std::ios_base::binary | std::ios_base::app); */
}

void Network::close_log_files() {
    out_log_file.close();
    in_log_file.close();
}

uint32_t Network::cast_raw_id(void* raw_id) {
    return (uint32_t)(uintptr_t) raw_id;
}
