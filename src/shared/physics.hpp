#pragma once
#include "flecs.h"
#include "raylib-cpp.hpp"

#include "components.hpp"
#include "const.hpp"


inline RayCollision get_ray_collision(Ray ray, flecs::entity e) {
    if (!e.has<ModelName>()) {
        assert(false);
        return RayCollision(false);
    }
    if (e.get<ModelName>().name == "3d_quad") {
        Vector3 pos = e.get<SimPosition>().val;
        Vector3 rot = e.get<SimRotation>().val;
        Vector3 scale = e.get<Scale>().val;
        Matrix m_scale = MatrixScale(scale.x, scale.y, scale.z);
        Matrix m_rot   = MatrixRotateXYZ((Vector3){rot.x, rot.y, rot.z});
        Matrix m_trans = MatrixTranslate(pos.x, pos.y, pos.z);

        Matrix transform = MatrixMultiply(MatrixMultiply(m_scale, m_rot), m_trans);
        Vector3 local_corners[4] = {
            {-0.5f, 0.0f, -0.5f}, // bottom-left
            { 0.5f, 0.0f, -0.5f}, // bottom-right
            { 0.5f, 0.0f,  0.5f}, // top-right
            {-0.5f, 0.0f,  0.5f}  // top-left
        };
        Vector3 world_corners[4];
        for (int i = 0; i < 4; i++) {
            world_corners[i] = Vector3Transform(local_corners[i], transform);
        }
        return GetRayCollisionQuad(ray, world_corners[0], world_corners[1], world_corners[2], world_corners[3]);
    }
    return RayCollision(false);
}

inline void process_collision(
    flecs::world& world,
    const raylib::Vector3& pos,
    raylib::Vector3& disp,
    bool& grounded)
{
    world.query<Terrain, ModelName>()
        .each([&] (flecs::entity e, Terrain, ModelName model_name) {
            RayCollision collision = get_ray_collision(Ray{pos, disp.Normalize()}, e);
            if (!collision.hit || collision.distance > disp.Length()) {
                if (!grounded) {
                    return;
                }
                // If no collision from moving, check if we're still close to ground
                collision = get_ray_collision(Ray{pos, {0, -1, 0}}, e);
                if (!collision.hit || collision.distance > 0.2) {
                    grounded = false;
                }
                return;
            }
            if (!grounded) {
                disp = (raylib::Vector3) collision.point - pos;
                disp *= 0.95;
                grounded = true;
            }
        }
    );
}

inline void update_gravity(float& grav, const bool& grounded) {
    if (!grounded) {
        grav += MOVE_UPDATE_RATE;
    }
    else {
        grav = 0;
    }
}

