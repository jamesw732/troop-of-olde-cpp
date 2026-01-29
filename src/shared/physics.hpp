#pragma once
#include <algorithm>
#include <limits>

#include "flecs.h"
#include "raylib.h"
#include "raymath.h"

#include "components.hpp"
#include "const.hpp"

/*
 * Check whether a ray collides with an entity's model, in simulation space
 */
inline RayCollision get_ray_collision(Ray ray, flecs::entity e) {
    if (!e.has<ModelType>()) {
        assert(false);
        return RayCollision{false};
    }
    Vector3 const pos = e.get<SimPosition>().val;
    Vector3 const rot = e.get<SimRotation>().val;
    Vector3 const scale = e.get<Scale>().val;
    Matrix const m_scale = MatrixScale(scale.x, scale.y, scale.z);
    Matrix const m_rot   = MatrixRotateXYZ((Vector3){rot.x, rot.y, rot.z});
    Matrix const m_trans = MatrixTranslate(pos.x, pos.y, pos.z);

    Matrix const transform = MatrixMultiply(MatrixMultiply(m_scale, m_rot), m_trans);
    if (e.get<ModelType>().name == "3d_quad") {
        Vector3 const local_corners[4] = {
            {-0.5F, 0.0F, -0.5F}, // bottom-left
            { 0.5F, 0.0F, -0.5F}, // bottom-right
            { 0.5F, 0.0F,  0.5F}, // top-right
            {-0.5F, 0.0F,  0.5F}  // top-left
        };
        Vector3 world_corners[4];
        for (int i = 0; i < 4; i++) {
            world_corners[i] = Vector3Transform(local_corners[i], transform);
        }
        return GetRayCollisionQuad(ray, world_corners[0], world_corners[1], world_corners[2], world_corners[3]);
    }
    if (e.get<ModelType>().name == "mesh") {
        // Just look at first mesh, may eventually need to use multiple
        return GetRayCollisionMesh(ray, *e.get<ModelPointer>().model->meshes, transform);
    }
    return RayCollision{false};
}

inline RayCollision find_closest_collision(
    flecs::world& world,
    const Vector3& pos,
    const Vector3& disp)
 {
    RayCollision closest_collision{};
    closest_collision.distance = std::numeric_limits<float>::max();
    // Find the closest collision, if there are multiple
    world.query<Terrain, ModelType>()
        .each([&] (flecs::entity e, const Terrain&, const ModelType&) {
            const RayCollision collision = get_ray_collision(Ray{pos, Vector3Normalize(disp)}, e);
            if(!collision.hit) return;
            if (closest_collision.distance > collision.distance) {
                closest_collision = collision;
            }
        });
    return closest_collision;
}

inline Vector3 get_wall_projection(
    const Vector3& direction,
    const Vector3& normal
)
{
    const Vector3& ret{normal.y, 0, -normal.x};
    return Vector3Normalize(Vector3Scale(ret, Vector3DotProduct(ret, direction)));
}

inline Vector3 get_slope_projection(
    const Vector3& direction,
    const Vector3& normal)
{
    return Vector3Normalize({
        direction.x * normal.y,
        -direction.z * normal.z - direction.x * normal.x,
        direction.z * normal.y
    });
}

inline Vector3 process_collision(
    flecs::world& world,
    const Vector3& position,
    const Vector3& direction,
    const float distance
)
{
    RayCollision closest_collision = find_closest_collision(world, position, direction);
    if (!closest_collision.hit || closest_collision.distance > distance) {
        return Vector3Scale(Vector3Normalize(direction), distance);
    }
    Vector3 new_point = Vector3Subtract(closest_collision.point, Vector3Scale(Vector3Normalize(direction), 0.1));
    float new_distance = distance - closest_collision.distance;
    Vector3 new_direction;
    if (Vector3Normalize(closest_collision.normal).y <= 0.2) {
        new_direction = get_wall_projection(direction, closest_collision.normal);
    }
    else {
        new_direction = get_slope_projection(direction, closest_collision.normal);
    }
    Vector3 new_displacement = Vector3Subtract(new_point, position);
    return Vector3Add(new_displacement, process_collision(world, new_point, new_direction, new_distance));
}

inline void check_beneath(
    flecs::world& world,
    const Vector3& pos,
    bool& grounded)
{
    RayCollision closest_collision{};
    closest_collision.distance = std::numeric_limits<float>::max();
    // Find the closest collision, if there are multiple
    world.query<Terrain, ModelType>()
        .each([&] (flecs::entity e, const Terrain&, const ModelType&) {
            const RayCollision collision = get_ray_collision(Ray{pos, {0, -1, 0}}, e);
            if(!collision.hit) return;
            if (closest_collision.distance > collision.distance) {
                closest_collision = collision;
            }
        });
    if (!closest_collision.hit || closest_collision.distance > 0.1) {
        grounded = false;
    }
    else {
        grounded = true;
    }
}

inline void update_gravity(float& grav, const bool& grounded) {
    if (!grounded) {
        grav -= MOVE_UPDATE_RATE;
    }
    else {
        grav = 0;
    }
}

