#include <iostream>
#include <vector>

#include "shared/packets.hpp"
#include "shared/serialize.hpp"
#include "shared/util.hpp"


std::vector<uint8_t> read_input_file(const std::string& filename) {
    std::ifstream input_file(filename, std::ios::binary);
    if (!input_file) {
        std::cerr << "Error opening file: " << filename << "\n";
        return {};
    }
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(input_file),
                                std::istreambuf_iterator<char>());
}


void deserialize_log(const Buffer& file_buffer) {
    size_t offset = 0;
    uint64_t timestamp;
    uint16_t packet_size = 0;
    while (offset + sizeof(timestamp) + sizeof(packet_size) <= file_buffer.size()) {
        std::memcpy(&timestamp, file_buffer.data() + offset, sizeof(timestamp));
        std::chrono::microseconds us(timestamp);
        std::chrono::system_clock::time_point tp(us);
        std::time_t time = std::chrono::system_clock::to_time_t(tp);
        std::tm* tm = std::localtime(&time);

        offset += sizeof(timestamp);

        std::memcpy(&packet_size, file_buffer.data() + offset, sizeof(uint16_t));
        offset += sizeof(uint16_t);

        if (offset + packet_size > file_buffer.size()) {
            std::cerr << "Corrupt log: packet exceeds buffer\n";
            return;
        }

        PacketType pkt_type = (PacketType) file_buffer[offset];
        offset += sizeof(PacketType);
        size_t payload_size = packet_size - sizeof(PacketType);

        const uint8_t* payload_start = file_buffer.data() + offset;
        Buffer payload_buffer(payload_start, payload_start + payload_size);
        bitsery::Deserializer<InputAdapter> des{payload_buffer.data(), payload_size};
        offset += payload_size;

        std::cout << std::put_time(tm, "%Y-%m-%d %H:%M:%S") << "\n";
        switch (pkt_type) {
            case PacketType::MovementInputPacket: {
                MovementInputPacket pkt;
                des.object(pkt);
                std::cout << pkt << "\n";
                break;
            }
            case PacketType::ClientLoginPacket: {
                ClientLoginPacket pkt;
                des.object(pkt);
                std::cout << pkt << "\n";
                break;
            }
            case PacketType::SpawnBatchPacket: {
                SpawnBatchPacket pkt;
                des.object(pkt);
                std::cout << pkt << "\n";
                break;
            }
            case PacketType::PlayerSpawnPacket: {
                PlayerSpawnPacket pkt;
                des.object(pkt);
                std::cout << pkt << "\n";
                break;
            }
            case PacketType::MovementUpdateBatchPacket: {
                MovementUpdateBatchPacket pkt;
                des.object(pkt);
                std::cout << pkt << "\n";
                break;
            }
            case PacketType::DisconnectPacket: {
                DisconnectPacket pkt;
                des.object(pkt);
                std::cout << pkt << "\n";
                break;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        return 0;
    }
    std::vector<uint8_t> buffer = read_input_file(argv[1]);
    deserialize_log(buffer);
}
