#pragma once

#include <vector>
#include <cstdint>
#include <fstream>
#include <string>

#include "raylib.h"

#include "mapgen-util.hpp"
#include "raymath.h"

const int GEN_ROOM_SIZE = 1;
const float WALL_REL_THICKNESS = 0.05;
const float WALL_THICKNESS = WALL_REL_THICKNESS * GEN_ROOM_SIZE;
const float WALL_REL_HEIGHT = 0.2f;
const float WALL_HEIGHT = WALL_REL_HEIGHT * GEN_ROOM_SIZE;
const float DOOR_REL_WIDTH = 0.3f;
const float DOOR_WIDTH = DOOR_REL_WIDTH * GEN_ROOM_SIZE;
const float DOOR_REL_HEIGHT = 0.8f;
const float DOOR_HEIGHT = DOOR_REL_HEIGHT * GEN_ROOM_SIZE;
const float HALF = 0.5f * GEN_ROOM_SIZE;
const float DOOR_HALF = 0.5f * DOOR_WIDTH;

inline bool has_direction(Direction directions, Direction direction)
{
    return (static_cast<uint8_t>(directions) &
            static_cast<uint8_t>(direction)) != 0;
}

struct Vertex
{
    Vector3 position;
    Vector3 normal;
    Vector2 texcoord;
};

inline void AddQuad(
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

inline void AddWall(
    std::vector<Vertex>& vertices,
    std::vector<uint16_t>& indices,
    bool has_door,
    Vector2 floor_offset
){
    auto make_point = [&] (Vector2 wall_offset) {
        if (floor_offset.x) {
            // make face along y, z in world axes
            return Vector3{HALF * floor_offset.x, WALL_HEIGHT * wall_offset.y, HALF * wall_offset.x};
        }
        return Vector3{HALF * wall_offset.x, WALL_HEIGHT * wall_offset.y, HALF * floor_offset.y};
    };
    Vector3 normal = {-floor_offset.x, 0, -floor_offset.y};
    if (!has_door) {
        AddQuad(
            vertices, indices,
            make_point({-1, 0}),
            make_point({-1, 1}),
            make_point({1, 1}),
            make_point({1, 0}),
            normal
        );
        return;
    } 
    // Left side of door wall
    AddQuad(
        vertices, indices,
        make_point({-1, 0}),
        make_point({-1, 1}),
        make_point({-DOOR_HALF, 1}),
        make_point({-DOOR_HALF, 0}),
        normal
    );
    // Right side of door wall
    AddQuad(
        vertices, indices,
        make_point({DOOR_HALF, 0}),
        make_point({DOOR_HALF, 1}),
        make_point({1, 1}),
        make_point({1, 0}),
        normal
    );
    // The overhang above the door
    AddQuad(
        vertices, indices,
        make_point({-DOOR_HALF, DOOR_HEIGHT}),
        make_point({-DOOR_HALF, 1}),
        make_point({DOOR_HALF, 1}),
        make_point({DOOR_HALF, DOOR_HEIGHT}),
        normal
    );
}

inline Mesh make_mesh(std::vector<Vertex> vertices, std::vector<uint16_t> indices) {
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

inline std::vector<Mesh> generate_room_meshes(Direction exits){
    std::vector<Mesh> ret{};
    
    // floor
    std::vector<Vertex> floor_vertices;
    std::vector<uint16_t> floor_indices;
    AddQuad(
        floor_vertices,
        floor_indices,
        {-HALF, 0.0f, -HALF},
        { HALF, 0.0f, -HALF},
        { HALF, 0.0f,  HALF},
        {-HALF, 0.0f,  HALF},
        {0.0f, 1.0f, 0.0f}
    );
    ret.push_back(make_mesh(floor_vertices, floor_indices));

    // walls
    std::vector<Vertex> interior_vertices;
    std::vector<uint16_t> interior_indices;
    AddWall(interior_vertices, interior_indices, has_direction(exits, Direction::Up), {0, -1});
    AddWall(interior_vertices, interior_indices, has_direction(exits, Direction::Down), {0, 1});
    AddWall(interior_vertices, interior_indices, has_direction(exits, Direction::Left), {-1, 0});
    AddWall(interior_vertices, interior_indices, has_direction(exits, Direction::Right), {1, 0});
    ret.push_back(make_mesh(interior_vertices, interior_indices));
    ret.push_back({});
    return ret;
}

inline void ExportRoomObj(
    const char* obj_path,
    const char* mtl_path,
    const Mesh& floor,
    const Mesh& interior,
    const Mesh& exterior)
{
    std::ofstream obj(obj_path);

    if (!obj)
        return;

    // OBJ references the MTL file by name.
    obj << "mtllib " << mtl_path << "\n";

    // Keep track of the OBJ vertex/UV/normal indices.
    // OBJ indices start at 1, unlike C/C++ indices.
    unsigned int vertex_offset = 1;

    auto write_mesh = [&](const Mesh& mesh, const char* material)
    {
        obj << "\nusemtl " << material << "\n";

        // Vertices
        for (int i = 0; i < mesh.vertexCount; ++i)
        {
            obj << "v "
                << mesh.vertices[i * 3 + 0] << " "
                << mesh.vertices[i * 3 + 1] << " "
                << mesh.vertices[i * 3 + 2] << "\n";
        }

        // Texture coordinates
        for (int i = 0; i < mesh.vertexCount; ++i)
        {
            obj << "vt "
                << mesh.texcoords[i * 2 + 0] << " "
                << mesh.texcoords[i * 2 + 1] << "\n";
        }

        // Normals
        for (int i = 0; i < mesh.vertexCount; ++i)
        {
            obj << "vn "
                << mesh.normals[i * 3 + 0] << " "
                << mesh.normals[i * 3 + 1] << " "
                << mesh.normals[i * 3 + 2] << "\n";
        }

        // Faces
        for (int i = 0; i < mesh.triangleCount; ++i)
        {
            unsigned int i0 = mesh.indices[i * 3 + 0] + vertex_offset;
            unsigned int i1 = mesh.indices[i * 3 + 1] + vertex_offset;
            unsigned int i2 = mesh.indices[i * 3 + 2] + vertex_offset;

            obj << "f "
                << i0 << "/" << i0 << "/" << i0 << " "
                << i1 << "/" << i1 << "/" << i1 << " "
                << i2 << "/" << i2 << "/" << i2 << "\n";
        }

        vertex_offset += mesh.vertexCount;
    };

    write_mesh(floor,    "floor");
    write_mesh(interior, "interior");
    write_mesh(exterior, "exterior");
}

inline Mesh CreateTestTriangle(
    Vector3 a,
    Vector3 b,
    Vector3 c)
{
    Mesh mesh = {0};

    mesh.vertexCount = 3;
    mesh.triangleCount = 1;

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
        MemAlloc(3 * sizeof(unsigned short))
    );

    Vector3 normal = Vector3Normalize(
        Vector3CrossProduct(
            Vector3Subtract(b, a),
            Vector3Subtract(c, a)
        )
    );

    Vector3 positions[] = {a, b, c};
    Vector2 texcoords[] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {0.5f, 1.0f}
    };

    for (int i = 0; i < 3; ++i)
    {
        mesh.vertices[i * 3 + 0] = positions[i].x;
        mesh.vertices[i * 3 + 1] = positions[i].y;
        mesh.vertices[i * 3 + 2] = positions[i].z;

        mesh.normals[i * 3 + 0] = normal.x;
        mesh.normals[i * 3 + 1] = normal.y;
        mesh.normals[i * 3 + 2] = normal.z;

        mesh.texcoords[i * 2 + 0] = texcoords[i].x;
        mesh.texcoords[i * 2 + 1] = texcoords[i].y;
    }

    mesh.indices[0] = 0;
    mesh.indices[1] = 1;
    mesh.indices[2] = 2;

    return mesh;
}

inline void ExportRoomMtl(const char* mtl_path) {
    std::ofstream mtl(mtl_path);

    if (!mtl)
        return;

    mtl << "newmtl floor\n";

    mtl << "newmtl interior\n";

    mtl << "newmtl exterior\n";
}

/*
 * Deprecated generation algorithm, boxy walls
 */
/* inline void AddBox( */
/*         std::vector<Vertex>& vertices, */
/*         std::vector<uint16_t>& indices, */
/*         Vector3 min, */
/*         Vector3 max) */
/* { */
/*     Vector3 p000{min.x, min.y, min.z}; */
/*     Vector3 p100{max.x, min.y, min.z}; */
/*     Vector3 p110{max.x, min.y, max.z}; */
/*     Vector3 p010{min.x, min.y, max.z}; */
/*     Vector3 p001{min.x, max.y, min.z}; */
/*     Vector3 p101{max.x, max.y, min.z}; */
/*     Vector3 p111{max.x, max.y, max.z}; */
/*     Vector3 p011{min.x, max.y, max.z}; */

/*     // Bottom (-Y) */
/*     AddQuad( */
/*             vertices, indices, */
/*             p000, p010, p110, p100, */
/*             {0.0f, -1.0f, 0.0f} */
/*            ); */

/*     // Top (+Y) */
/*     AddQuad( */
/*             vertices, indices, */
/*             p001, p101, p111, p011, */
/*             {0.0f, 1.0f, 0.0f} */
/*            ); */

/*     // North / back (-Z) */
/*     AddQuad( */
/*             vertices, indices, */
/*             p000, p100, p101, p001, */
/*             {0.0f, 0.0f, -1.0f} */
/*            ); */

/*     // South / front (+Z) */
/*     AddQuad( */
/*             vertices, indices, */
/*             p010, p011, p111, p110, */
/*             {0.0f, 0.0f, 1.0f} */
/*            ); */

/*     // West / left (-X) */
/*     AddQuad( */
/*             vertices, indices, */
/*             p000, p001, p011, p010, */
/*             {-1.0f, 0.0f, 0.0f} */
/*            ); */

/*     // East / right (+X) */
/*     AddQuad( */
/*             vertices, indices, */
/*             p100, p110, p111, p101, */
/*             {1.0f, 0.0f, 0.0f} */
/*            ); */
/* } */


/* inline void AddBoxWall( */
/*         std::vector<Vertex>& vertices, */
/*         std::vector<uint16_t>& indices, */
/*         float half_size, */
/*         float wall_height, */
/*         float door_width, */
/*         bool has_door, */
/*         bool horizontal, */
/*         float fixed_position, */
/*         bool positive_side) */
/* { */
/*     const float door_half = door_width * 0.5f; */

/*     // Coordinates along the wall. */
/*     float left = -half_size; */
/*     float right = half_size; */

/*     int protrude_direction = positive_side ? -1 : 1; */

/*     auto make_point = [&](float along, float height, float protrude) */
/*     { */
/*         if (horizontal) */
/*         { */
/*             return Vector3{ */
/*                 along, */
/*                     height, */
/*                     fixed_position + protrude_direction * protrude */
/*             }; */
/*         } */
/*         else */
/*         { */
/*             return Vector3{ */
/*                 fixed_position + protrude_direction * protrude, */
/*                                height, */
/*                                along */
/*             }; */
/*         } */
/*     }; */

/*     if (!has_door) */
/*     { */
/*         AddBox( */
/*             vertices, */
/*             indices, */
/*             make_point(left, 0.0f, 0), */
/*             make_point(right, wall_height, WALL_REL_THICKNESS) */
/*         ); */

/*         return; */
/*     } */

/*     /* */
/*        Bottom left → bottom right doorway sections. */

/*        Left segment */
/*        *1/ */
/*     if (left < -door_half) */
/*     { */
/*         AddBox( */
/*             vertices, */
/*             indices, */
/*             make_point(left, 0.0f, 0), */
/*             make_point(-door_half, wall_height, WALL_REL_THICKNESS) */
/*         ); */
/*     } */

/*     /* */
/*        Right segment */
/*        *1/ */
/*     if (door_half < right) */
/*     { */
/*         AddBox( */
/*             vertices, */
/*             indices, */
/*             make_point(door_half, 0.0f, 0), */
/*             make_point(right, wall_height, WALL_REL_THICKNESS) */
/*         ); */
/*     } */

/*     /* */
/*        Wall above the doorway. */
/*        This is the lintel/header above the opening. */
/*        *1/ */
/*     AddBox( */
/*             vertices, */
/*             indices, */
/*             make_point(-door_half, wall_height * 0.8f, 0), */
/*             make_point(door_half, wall_height, WALL_REL_THICKNESS) */
/*           ); */
/*     /* */
/*        The portion below the doorway is intentionally absent. */
/*        The doorway extends all the way down to the floor. */
/*        *1/ */
/* } */



/* /* */
/*  * Deprecated. Generates box-shaped walls, but puts everything in the same mesh and as such is incompatible with texturing */
/*  *1/ */
/* inline Mesh generate_room_mesh( */
/*     Direction exits, */
/*     float room_size = GEN_ROOM_SIZE, */
/*     float wall_height = 0.2f * GEN_ROOM_SIZE, */
/*     float door_width = 0.15f * GEN_ROOM_SIZE */
/* ) { */
/*     std::vector<Vertex> vertices; */
/*     std::vector<uint16_t> indices; */

/*     const float half = room_size * 0.5f; */

/*     // floor */
/*     AddQuad( */
/*         vertices, */
/*         indices, */

/*         // Counter-clockwise when viewed from above. */
/*         {-half, 0.0f, -half}, */
/*         { half, 0.0f, -half}, */
/*         { half, 0.0f,  half}, */
/*         {-half, 0.0f,  half}, */

/*         {0.0f, 1.0f, 0.0f} */
/*     ); */

/*     // north */
/*     AddBoxWall( */
/*         vertices, */
/*         indices, */
/*         half, */
/*         wall_height, */
/*         door_width, */
/*         has_direction(exits, Direction::Up), */
/*         true,       // horizontal */
/*         -half,      // Z position */
/*         false       // outward normal = -Z */
/*     ); */

/*     // south */
/*     AddBoxWall( */
/*         vertices, */
/*         indices, */
/*         half, */
/*         wall_height, */
/*         door_width, */
/*         has_direction(exits, Direction::Down), */
/*         true,       // horizontal */
/*         half,       // Z position */
/*         true        // outward normal = +Z */
/*     ); */

/*     // west */
/*     AddBoxWall( */
/*         vertices, */
/*         indices, */
/*         half, */
/*         wall_height, */
/*         door_width, */
/*         has_direction(exits, Direction::Left), */
/*         false,      // vertical */
/*         -half,      // X position */
/*         false       // outward normal = -X */
/*     ); */

/*     // east */
/*     AddBoxWall( */
/*         vertices, */
/*         indices, */
/*         half, */
/*         wall_height, */
/*         door_width, */
/*         has_direction(exits, Direction::Right), */
/*         false,      // vertical */
/*         half,       // X position */
/*         true        // outward normal = +X */
/*     ); */


/*     // Convert to raylib mesh */
/*     Mesh mesh = {0}; */

/*     mesh.vertexCount = static_cast<int>(vertices.size()); */
/*     mesh.triangleCount = static_cast<int>(indices.size() / 3); */

/*     mesh.vertices = static_cast<float*>( */
/*         MemAlloc(mesh.vertexCount * 3 * sizeof(float)) */
/*     ); */

/*     mesh.normals = static_cast<float*>( */
/*         MemAlloc(mesh.vertexCount * 3 * sizeof(float)) */
/*     ); */

/*     mesh.texcoords = static_cast<float*>( */
/*         MemAlloc(mesh.vertexCount * 2 * sizeof(float)) */
/*     ); */

/*     mesh.indices = static_cast<unsigned short*>( */
/*         MemAlloc(indices.size() * sizeof(unsigned short)) */
/*     ); */

/*     for (int i = 0; i < mesh.vertexCount; ++i) */
/*     { */
/*         mesh.vertices[i * 3 + 0] = vertices[i].position.x; */
/*         mesh.vertices[i * 3 + 1] = vertices[i].position.y; */
/*         mesh.vertices[i * 3 + 2] = vertices[i].position.z; */

/*         mesh.normals[i * 3 + 0] = vertices[i].normal.x; */
/*         mesh.normals[i * 3 + 1] = vertices[i].normal.y; */
/*         mesh.normals[i * 3 + 2] = vertices[i].normal.z; */

/*         mesh.texcoords[i * 2 + 0] = vertices[i].texcoord.x; */
/*         mesh.texcoords[i * 2 + 1] = vertices[i].texcoord.y; */
/*     } */

/*     for (size_t i = 0; i < indices.size(); ++i) */
/*     { */
/*         mesh.indices[i] = indices[i]; */
/*     } */

/*     return mesh; */
/* } */

