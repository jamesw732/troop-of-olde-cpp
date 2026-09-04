#include "network.hpp"
#include "entities.hpp"
#include "login.hpp"
#include "../shared/packets.hpp"


/*
 * This struct acts as a shallow boundary between networking data and ECS data
 * Game logic should be extremely minimal in this file
 * The desired pattern is to store received networking data in "intermediary" ECS components,
 * to be processed later in a system like any other component would.
 */
struct PacketHandler {
    flecs::world& world;
    LoginHandler& login_handler;
    std::unordered_map<uint32_t, flecs::entity>& client_id_to_entity;

    void handle_packets(std::vector<RecvPacket>& packets) {
        for (RecvPacket packet: packets) {
            handle_packet(packet);
        }
        packets.clear();
    }

    void handle_packet(RecvPacket packet) {
        // This function should not do any complicated operations, those should be done by systems
        // Tentative heuristic is that if we need to query for a component, it's too complicated
        uint8_t* buffer = packet.data.data();
        size_t size = packet.data.size();
        bitsery::Deserializer<InputAdapter> des{InputAdapter{buffer, size}};
        PacketType pkt_type;
        des.value1b(pkt_type);
        switch (pkt_type) {
            case PacketType::MovementInputPacket: {
                if (!client_id_to_entity.contains(packet.client_id)) return;
                flecs::entity player = client_id_to_entity[packet.client_id];
                MovementInputPacket input_packet;
                des.object(input_packet);
                player.set<RecvMoveTick>({input_packet.tick});
                player.set<InputBuffer>({input_packet.size, input_packet.inputs});
                break;
            }

            case PacketType::LoginRequestPacket: {
                LoginRequestPacket login;
                des.object(login);
                login_handler.logins.push_back({packet.client_id, login});
                break;
            }

            case PacketType::DisconnectPacket: {
                flecs::entity player = client_id_to_entity[packet.client_id];
                player.add<Disconnected>();
            }

            default: {
                break;
            }
        }

    }

};
