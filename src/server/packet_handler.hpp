#include "flecs.h"

#include "network.hpp"
#include "entities.hpp"

class PacketHandler {
  private:
    flecs::world world;
  public:
    PacketHandler(flecs::world& w) : world(w) {};

    void handle_packets(std::deque<RecvPacket>& packets) {
        while (true) {
            if (packets.empty()) {
                return;
            }
            auto packet_data = packets.front();
            handle_packet(packet_data);
            packets.pop_front();
        }
    }

    void handle_packet(RecvPacket packet) {
        flecs::entity player = packet.e;
        uint8_t* buffer = packet.packet_data.data();
        size_t size = packet.packet_data.size();
        bitsery::Deserializer<InputAdapter> des{InputAdapter{buffer, size}};
        PacketType pkt_type;
        des.value1b(pkt_type);
        switch (pkt_type) {
            case PacketType::MovementInputPacket: {
                MovementInputPacket input_packet;
                des.object(input_packet);
                // TODO: Consider reworking packet handling to not store containers in ECS table
                player.set<MovementInputPacket>(input_packet);
                break;
            }

            case PacketType::ClientLoginPacket: {
                ClientLoginPacket login;
                des.object(login);
                add_character_components(player);
                player.set<DisplayName>({login.name});
                player.set<SimPosition>({login.pos});
                player.set<SimRotation>({login.rot});
                player.add<NeedsSpawnBatch>();
                break;
            }

            default: {
                break;
            }
        }

    }

};
