#include "../shared/pch-networking.hpp"

#include "network.hpp"
#include "../shared/network_components.hpp"
#include "../shared/const.hpp"
#include "../shared/util.hpp"


struct NetworkImpl {
    ENetHost* server = {0};
    ENetAddress address = {0};
    ENetEvent event;
    std::unordered_map<NetworkId, ENetPeer*> netid_to_peer;
};

Network::Network(flecs::world& w) : world(w), impl(std::make_unique<NetworkImpl>()) {
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
                auto entity = world.entity();
                entity.set<NetworkId>({network_id_counter});
                impl->netid_to_peer[{network_id_counter}] = impl->event.peer;
                network_id_counter++;
                impl->event.peer->data = (void*) entity.raw_id();
                break;
            }

            case ENET_EVENT_TYPE_RECEIVE: {
                auto id = cast_raw_id(impl->event.peer->data);
                flecs::entity entity(world, id);
                uint8_t* buffer = impl->event.packet->data;
                size_t size = impl->event.packet->dataLength;
                Buffer packet_data(buffer, buffer + size);
                RecvPacket recv_packet{entity, packet_data};
                // To be handled by PacketHandler
                packets.push_back(recv_packet);
                dbg("Received event");
                log(in_log_file, packet_data, size);
                break;
            }

            case ENET_EVENT_TYPE_DISCONNECT: {
                std::cout << "Client with Entity id "
                    << cast_raw_id(impl->event.peer->data)
                    << " disconnected."
                    << std::endl;
                auto id = cast_raw_id(impl->event.peer->data);
                flecs::entity entity(world, id);
                entity.add<Disconnected>();
                impl->event.peer->data = NULL;
                enet_peer_reset(impl->event.peer);
                break;
            }

            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
                std::cout << "Client with Entity id "
                    << cast_raw_id(impl->event.peer->data)
                    << " disconnected due to timeout."
                    << std::endl;
                auto id = cast_raw_id(impl->event.peer->data);
                flecs::entity entity(world, id);
                entity.add<Disconnected>();
                impl->event.peer->data = NULL;
                enet_peer_reset(impl->event.peer);
                break;
            }

            case ENET_EVENT_TYPE_NONE:
                break;
        }
    }
}

void Network::queue_data_unreliable(const NetworkId& network_id, const Buffer& buffer, const size_t size){
    auto netid_peer = impl->netid_to_peer.find(network_id);
    if (netid_peer == impl->netid_to_peer.end()) {
        dbg("Failed to find peer");
        return;
    }
    ENetPeer* peer = netid_peer->second;
    ENetPacket* packet = enet_packet_create(buffer.data(), size, 0);
    enet_peer_send(peer, 0, packet);
    log(out_log_file, buffer, size);
}

void Network::queue_data_reliable(const NetworkId& network_id, const Buffer& buffer, const size_t size){
    auto netid_peer = impl->netid_to_peer.find(network_id);
    if (netid_peer == impl->netid_to_peer.end()) {
        dbg("Failed to find peer");
        return;
    }
    ENetPeer* peer = netid_peer->second;
    ENetPacket* packet = enet_packet_create(buffer.data(), size, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 1, packet);
    log(out_log_file, buffer, size);
}

void Network::send_network_buffer() {
    enet_host_flush(impl->server);
}

void Network::open_log_files() {
    out_log_file.open("server-out.bin", std::ios_base::binary | std::ios_base::app);
    in_log_file.open("server-in.bin", std::ios_base::binary | std::ios_base::app);
}

void Network::close_log_files() {
    out_log_file.close();
    in_log_file.close();
}

ecs_entity_t Network::cast_raw_id(void* raw_id) {
    return (ecs_entity_t)(uintptr_t) raw_id;
}
