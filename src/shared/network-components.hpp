#pragma once
#include <cstdint>
#include <functional>


struct ClientId {
    uint32_t id;

    bool operator==(const ClientId& other) const noexcept {
        return id == other.id;
    }
};

namespace std {
    template<>
    struct hash<ClientId> {
        std::size_t operator()(const ClientId& nid) const noexcept {
            return std::hash<uint32_t>{}(nid.id);
        }
    };
}

struct Disconnected {};
