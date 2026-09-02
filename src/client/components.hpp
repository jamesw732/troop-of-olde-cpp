#pragma once
#include <cstdint>
#include <unordered_map>

#include "raylib.h"
#include "flecs.h"


struct LocalPlayer {};

struct PrevPredPosition {
    Vector3 val{};
};

struct RenderPosition {
    Vector3 val{};
};

struct RenderRotation {
    Vector3 val{};
};

struct PrevPredRotation {
    Vector3 val{};
};

struct LerpTimer {
    float val = 0;
};

struct AckTick {
    uint16_t val{static_cast<uint16_t>(0)};
};

struct RecvAckTick {
    uint16_t val{static_cast<uint16_t>(0)};
};

struct RecvPosition {
    Vector3 val{};
};

struct RecvRotation {
    Vector3 val{};
};

struct RecvGravity {
    float val;
};

struct RecvGrounded {
    bool val;
};

struct NetworkMap {
    std::unordered_map<uint32_t, flecs::entity> netid_to_entity;
};

struct CameraInput {
    int8_t rot_x = 0; // Up/Down arrow rotation
    Vector2 free_rot{}; // Camera's free rotation about character, not tied to character's real rotation
    bool reset = false;
    float scroll = 0;
};

struct CamRotation {
    float x = 0;
    float y = 0;
};

struct CamDistance {
    float val = 10;
};
