#include "entities.hpp"
#include "network.hpp"
#include "../shared/packets.hpp"
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
                // We just entered world and need world's whole state
                SpawnBatchPacket spawn_batch;
                des.object(spawn_batch);
                for (PlayerSpawnState spawn_state: spawn_batch.spawn_states) {
                    flecs::entity entity;
                    if (spawn_state.network_id == spawn_batch.local_player_id) {
                        entity = create_local_player(world);
                    }
                    else {
                        entity = create_remote_player(world);
                    }
                    entity.set<PredPosition>({spawn_state.pos});
                    entity.set<RenderPosition>({spawn_state.pos});
                    entity.set<PrevPredPosition>({spawn_state.pos});
                    entity.set<PredRotation>({spawn_state.rot});
                    entity.set<RenderRotation>({spawn_state.rot});
                    entity.set<PrevPredRotation>({spawn_state.rot});
                    entity.set<NetworkId>({spawn_state.network_id});
                    entity.set<DisplayName>({spawn_state.name});
                    entity.set<CamRotation>({30.0});
                    entity.set<Color>(RED);
                    entity.set<ModelType>({"cube"});
                    netid_to_entity[spawn_state.network_id] = entity;
                }
                // std::cout << "Batch Spawn Packet: " << '\n';
                // for (auto pair: netid_to_entity) {
                //     std::cout << (int) pair.first.id << ", " << (int) pair.second << '\n';
                // }
                break;
            }

            case PacketType::PlayerSpawnPacket: {
                // Remote player entered world and we need that new player's state
                PlayerSpawnPacket spawn_packet;
                des.object(spawn_packet);
                PlayerSpawnState spawn_state = spawn_packet.spawn_state;
                flecs::entity entity = create_remote_player(world);
                entity.set<NetworkId>({spawn_state.network_id});
                entity.set<PredPosition>({spawn_state.pos});
                entity.set<PredRotation>({spawn_state.rot});
                entity.set<PrevPredPosition>({spawn_state.pos});
                entity.set<PrevPredRotation>({spawn_state.rot});
                entity.set<RenderPosition>({spawn_state.pos});
                entity.set<RenderRotation>({spawn_state.rot});
                entity.set<DisplayName>({spawn_state.name});
                entity.set<Color>(RED);
                entity.set<ModelType>({"cube"});
                netid_to_entity[spawn_state.network_id] = entity;
                // std::cout << "Single Spawn Packet: " << '\n';
                // std::cout << spawn_state.network_id.id << '\n';
                // for (auto pair: netid_to_entity) {
                //     std::cout <<  pair.first.id << ", " << pair.second << '\n';
                // }
                break;
            }

            case PacketType::MovementUpdateBatchPacket: {
                MovementUpdateBatchPacket batch;
                des.object(batch);
#ifndef DISABLE_SERVER
                auto& netid_to_entity = world.get_mut<NetworkMap>().netid_to_entity;
                for (MovementUpdate move_update: batch.move_updates) {
                    auto netid_entity = netid_to_entity.find(move_update.network_id);
                    if (netid_entity == netid_to_entity.end()) {
                        continue;
                    }
                    flecs::entity e = netid_entity->second;
                    if ((int16_t) (move_update.ack_tick - e.get<AckTick>().val) <= 0) {
                        continue;
                    }
                    e.set<AckTick>({move_update.ack_tick});
                    e.set<SimPosition>({move_update.pos});
                    e.set<SimRotation>({move_update.rot});
                    e.set<SimGravity>({move_update.gravity});
                    e.set<SimGrounded>({move_update.grounded});
                }
#endif
                break;
            }

            case PacketType::DisconnectPacket: {
                DisconnectPacket dc_packet;
                des.object(dc_packet);
                std::cout << dc_packet.network_id << '\n';
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
