#pragma once

#include <vector>
#include <cstdint>

#include "raylib.h"

#include "mapgen-util.hpp"

inline bool has_direction(Direction directions, Direction direction)
{
    return (static_cast<uint8_t>(directions) &
            static_cast<uint8_t>(direction)) != 0;
}

namespace
{
    struct Vertex
    {
        Vector3 position;
        Vector3 normal;
        Vector2 texcoord;
    };

    void AddQuad(
        std::vector<Vertex>& vertices,
        std::vector<uint16_t>& indices,
        Vector3 a,
        Vector3 b,
        Vector3 c,
        Vector3 d,
        Vector3 normal)
    {
        uint16_t base = static_cast<uint16_t>(vertices.size());

        vertices.push_back({a, normal, {0.0f, 0.0f}});
        vertices.push_back({b, normal, {1.0f, 0.0f}});
        vertices.push_back({c, normal, {1.0f, 1.0f}});
        vertices.push_back({d, normal, {0.0f, 1.0f}});

        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);

        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }

    /*
        Adds a wall along one axis with a centered doorway.

        The wall extends from -half_size to +half_size.

        For a wall with a doorway:

             wall
        ┌───────────────┐
        │               │
        │     ┌───┐     │
        │     │   │     │
        └─────┘   └─────┘
              door

        The wall is represented by:
            left wall segment
            right wall segment
            wall above the doorway
    */
    void AddWallWithDoor(
        std::vector<Vertex>& vertices,
        std::vector<uint16_t>& indices,
        float half_size,
        float wall_height,
        float door_width,
        bool has_door,
        bool horizontal,
        float fixed_position,
        bool positive_side)
    {
        const float door_half = door_width * 0.5f;

        // Coordinates along the wall.
        float left = -half_size;
        float right = half_size;

        /*
            Horizontal walls run along X.

            Vertical walls run along Z.

            `positive_side` determines which side of the room
            the wall is on, which determines the normal.
        */

        Vector3 normal{0};

        if (horizontal)
        {
            normal = {
                0.0f,
                0.0f,
                positive_side ? 1.0f : -1.0f
            };
        }
        else
        {
            normal = {
                positive_side ? 1.0f : -1.0f,
                0.0f,
                0.0f
            };
        }

        auto make_point = [&](float along, float height)
        {
            if (horizontal)
            {
                return Vector3{
                    along,
                    height,
                    fixed_position
                };
            }
            else
            {
                return Vector3{
                    fixed_position,
                    height,
                    along
                };
            }
        };

        if (!has_door)
        {
            AddQuad(
                vertices,
                indices,
                make_point(left, 0.0f),
                make_point(right, 0.0f),
                make_point(right, wall_height),
                make_point(left, wall_height),
                normal
            );

            return;
        }

        /*
            Bottom left → bottom right doorway sections.

            Left segment
        */
        if (left < -door_half)
        {
            AddQuad(
                vertices,
                indices,
                make_point(left, 0.0f),
                make_point(-door_half, 0.0f),
                make_point(-door_half, wall_height),
                make_point(left, wall_height),
                normal
            );
        }

        /*
            Right segment
        */
        if (door_half < right)
        {
            AddQuad(
                vertices,
                indices,
                make_point(door_half, 0.0f),
                make_point(right, 0.0f),
                make_point(right, wall_height),
                make_point(door_half, wall_height),
                normal
            );
        }

        /*
            Wall above the doorway.

            This is the lintel/header above the opening.
        */
        AddQuad(
            vertices,
            indices,
            make_point(-door_half, wall_height * 0.8f),
            make_point(door_half, wall_height * 0.8f),
            make_point(door_half, wall_height),
            make_point(-door_half, wall_height),
            normal
        );

        /*
            The portion below the doorway is intentionally absent.
            The doorway extends all the way down to the floor.
        */
    }
}


inline Mesh generate_room_mesh(
    Direction exits,
    float room_size = ROOM_SIZE,
    float wall_height = 2.0f,
    float door_width = 1.5f
) {
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    const float half = room_size * 0.5f;

    /*
        --------------------------------------------------
        FLOOR
        --------------------------------------------------
    */

    AddQuad(
        vertices,
        indices,

        // Counter-clockwise when viewed from above.
        {-half, 0.0f, -half},
        { half, 0.0f, -half},
        { half, 0.0f,  half},
        {-half, 0.0f,  half},

        {0.0f, 1.0f, 0.0f}
    );

    /*
        --------------------------------------------------
        NORTH WALL (-Z)
        --------------------------------------------------
    */

    AddWallWithDoor(
        vertices,
        indices,
        half,
        wall_height,
        door_width,
        has_direction(exits, Direction::Up),
        true,       // horizontal
        -half,      // Z position
        false       // outward normal = -Z
    );

    /*
        --------------------------------------------------
        SOUTH WALL (+Z)
        --------------------------------------------------
    */

    AddWallWithDoor(
        vertices,
        indices,
        half,
        wall_height,
        door_width,
        has_direction(exits, Direction::Down),
        true,       // horizontal
        half,       // Z position
        true        // outward normal = +Z
    );

    /*
        --------------------------------------------------
        WEST WALL (-X)
        --------------------------------------------------
    */

    AddWallWithDoor(
        vertices,
        indices,
        half,
        wall_height,
        door_width,
        has_direction(exits, Direction::Left),
        false,      // vertical
        -half,      // X position
        false       // outward normal = -X
    );

    /*
        --------------------------------------------------
        EAST WALL (+X)
        --------------------------------------------------
    */

    AddWallWithDoor(
        vertices,
        indices,
        half,
        wall_height,
        door_width,
        has_direction(exits, Direction::Right),
        false,      // vertical
        half,       // X position
        true        // outward normal = +X
    );

    /*
        --------------------------------------------------
        CONVERT TO RAYLIB MESH
        --------------------------------------------------
    */

    Mesh mesh = {0};

    mesh.vertexCount = static_cast<int>(vertices.size());
    mesh.triangleCount = static_cast<int>(indices.size() / 3);

    mesh.vertices = static_cast<float*>(
        MemAlloc(mesh.vertexCount * 3 * sizeof(float))
    );

    mesh.normals = static_cast<float*>(
        MemAlloc(mesh.vertexCount * 3 * sizeof(float))
    );

    mesh.texcoords = static_cast<float*>(
        MemAlloc(mesh.vertexCount * 2 * sizeof(float))
    );

    mesh.indices = static_cast<unsigned short*>(
        MemAlloc(indices.size() * sizeof(unsigned short))
    );

    for (int i = 0; i < mesh.vertexCount; ++i)
    {
        mesh.vertices[i * 3 + 0] = vertices[i].position.x;
        mesh.vertices[i * 3 + 1] = vertices[i].position.y;
        mesh.vertices[i * 3 + 2] = vertices[i].position.z;

        mesh.normals[i * 3 + 0] = vertices[i].normal.x;
        mesh.normals[i * 3 + 1] = vertices[i].normal.y;
        mesh.normals[i * 3 + 2] = vertices[i].normal.z;

        mesh.texcoords[i * 2 + 0] = vertices[i].texcoord.x;
        mesh.texcoords[i * 2 + 1] = vertices[i].texcoord.y;
    }

    for (size_t i = 0; i < indices.size(); ++i)
    {
        mesh.indices[i] = indices[i];
    }

    return mesh;
}
