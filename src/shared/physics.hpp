#pragma once
#include <limits>

#include "flecs.h"
#include "raylib.h"
#include "raymath.h"

#include "components.hpp"
#include "const.hpp"
#include "raylib-util.hpp"

inline Matrix get_transformation_matrix(const Vector3 pos, const Vector3 rot, const Vector3 scale) {
    Matrix const m_scale = MatrixScale(scale.x, scale.y, scale.z);
    Matrix const m_rot   = MatrixRotateXYZ({rot.x, rot.y, rot.z});
    Matrix const m_trans = MatrixTranslate(pos.x, pos.y, pos.z);
    return MatrixMultiply(MatrixMultiply(m_scale, m_rot), m_trans);
}

inline RayCollision find_closest_collision(
    flecs::world& world,
    const Vector3& pos,
    const Vector3& dir)
 {
    RayCollision closest_collision{
        .hit=false,
        .distance=std::numeric_limits<float>::max(),
        .point={0, 0, 0},
        .normal={0, 1, 0}
    };
    // Find the closest collision, if there are multiple
    world.query<Terrain, ModelPointer, SimPosition, SimRotation, Scale>()
        .each([&] (
            Terrain, const ModelPointer& model,
            SimPosition mesh_pos, SimRotation mesh_rot, Scale mesh_scale)
        {
            Ray ray{pos, Vector3Normalize(dir)};
            Matrix transform = get_transformation_matrix(mesh_pos.val, mesh_rot.val, mesh_scale.val);
            const RayCollision collision = GetRayCollisionMesh(ray, *model.model->meshes, transform);
            if(!collision.hit || closest_collision.distance <= collision.distance) return;
            closest_collision = collision;
        }
    );
    return closest_collision;
}

inline Vector3 get_wall_projection(
    const Vector3& direction,
    const Vector3& normal
)
{
    const Vector3 ret{normal.z, 0, -normal.x};
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
    const float distance,
    int depth = 1,
    int max_depth = 3
)
{
    // TODO: Figure out what to do about clipping through negative-incline surfaces
    if (depth > max_depth) {
        return Vector3{};
    }
    RayCollision closest_collision = find_closest_collision(world, position, direction);
    if (!closest_collision.hit || closest_collision.distance > distance) {
        return Vector3Scale(Vector3Normalize(direction), distance);
    }
    Vector3 cur_displacement = Vector3Scale(Vector3Normalize(direction), closest_collision.distance);
    // Add a small damping factor with constant magnitude
    Vector3 damping = Vector3Scale(Vector3Normalize(direction), 0.01);
    cur_displacement = Vector3Subtract(cur_displacement, damping);
    Vector3 new_point = Vector3Add(position, cur_displacement);

    float rem_distance = distance - closest_collision.distance;
    Vector3 new_direction;
    if (Vector3Normalize(closest_collision.normal).y <= 0.2) {
        new_direction = get_wall_projection(direction, closest_collision.normal);
    }
    else {
        new_direction = get_slope_projection(direction, closest_collision.normal);
    }
    return Vector3Add(
        cur_displacement,
        process_collision(world, new_point, new_direction, rem_distance, depth + 1));
}

inline void check_beneath(
    flecs::world& world,
    const Vector3& pos,
    bool& grounded)
{
    RayCollision closest_collision = find_closest_collision(world, pos, {0, -1, 0});
    if (!closest_collision.hit || closest_collision.distance > 0.01) {
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

