#include "entities.hpp"
#include "network.hpp"
#include "../shared/serialize.hpp"


class PacketHandler {
  public:
    flecs::world world;
    std::unordered_map<NetworkId, flecs::entity> netid_to_entity;

    PacketHandler(flecs::world& w) : world(w) {};

    void handle_packets(std::deque<std::vector<uint8_t>>& packets) {
        while (true) {
            if (packets.empty()) {
                return;
            }
            auto packet_data = packets.front();
            handle_packet(packet_data);
            packets.pop_front();
        }
    }

    void handle_packet(std::vector<uint8_t>& packet_data) {
        bitsery::Deserializer<InputAdapter> des{InputAdapter{packet_data.data(), packet_data.size()}};
        PacketType pkt_type;
        des.value1b(pkt_type);
        auto& netid_to_entity = world.get_mut<NetworkMap>().netid_to_entity;
        switch (pkt_type) {
            case PacketType::SpawnBatchPacket: {
                // We just entered world
                dbg("Received batch spawn packet");
                SpawnBatchPacket spawn_batch;
                des.object(spawn_batch);
                dbg(spawn_batch.local_player_id.id);
                for (PlayerSpawnState spawn_state: spawn_batch.spawn_states) {
                    flecs::entity entity;
                    if (spawn_state.network_id.id == spawn_batch.local_player_id.id) {
                        entity = create_local_player(world);
                    }
                    else {
                        entity = create_remote_player(world);
                    }
                    entity.set<SimPosition>(spawn_state.pos);
                    entity.set<RenderPosition>({spawn_state.pos.val});
                    entity.set<PrevSimPosition>({spawn_state.pos.val});
                    entity.set<NetworkId>(spawn_state.network_id);
                    entity.set<DisplayName>(spawn_state.name);
                    netid_to_entity[spawn_state.network_id] = entity;
                }
                // std::cout << "Batch Spawn Packet: " << '\n';
                // for (auto pair: netid_to_entity) {
                //     std::cout << (int) pair.first.id << ", " << (int) pair.second << '\n';
                // }
                break;
            }

            case PacketType::PlayerSpawnPacket: {
                dbg("Received external spawn packet");
                // Remote player entered world
                PlayerSpawnPacket spawn_packet;
                des.object(spawn_packet);
                PlayerSpawnState spawn_state = spawn_packet.spawn_state;
                flecs::entity entity = create_remote_player(world);
                entity.set<RenderPosition>({spawn_state.pos.val});
                entity.set<SimPosition>(spawn_state.pos);
                entity.set<PrevSimPosition>({spawn_state.pos.val});
                entity.set<NetworkId>(spawn_state.network_id);
                dbg(spawn_state.network_id.id);
                entity.set<DisplayName>(spawn_state.name);
                netid_to_entity[spawn_state.network_id] = entity;
                // std::cout << "Single Spawn Packet: " << '\n';
                // std::cout << spawn_state.network_id.id << '\n';
                // for (auto pair: netid_to_entity) {
                //     std::cout <<  pair.first.id << ", " << pair.second << '\n';
                // }
                break;
            }

            case PacketType::MovementUpdateBatchPacket: {
                dbg("Received movement batch packet");
                MovementUpdateBatchPacket batch;
                des.object(batch);
                world.set<MovementUpdateBatchPacket>(batch);
                break;
            }

            case PacketType::DisconnectPacket: {
                dbg("Received disconnect packet");
                DisconnectPacket dc_packet;
                des.object(dc_packet);
                std::cout << dc_packet.network_id.id << '\n';
                auto netid_entity = netid_to_entity.find(dc_packet.network_id);
                if (netid_entity == netid_to_entity.end()) {
                    break;
                }
                flecs::entity entity = netid_entity->second;
                entity.add<Disconnected>();
                break;
            }

            default: {
                break;
            }
        }
    }

};
