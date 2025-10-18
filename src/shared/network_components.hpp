#pragma once
#include <cstdint>
#include <functional>


struct NetworkId {
    uint32_t id;

    bool operator==(const NetworkId& other) const noexcept {
        return id == other.id;
    }
};

namespace std {
    template<>
    struct hash<NetworkId> {
        std::size_t operator()(const NetworkId& nid) const noexcept {
            return std::hash<uint32_t>{}(nid.id);
        }
    };
}

struct Disconnected {};
