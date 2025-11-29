#include <iostream>
#include <cstring>

#include "raylib.h"
#include "external/cgltf.h"

#include "mesh_loader.hpp"

Mesh* LoadMesh(const char* file_name) {
    cgltf_options options{};
    cgltf_data* data = NULL;
    cgltf_result res = cgltf_parse_file(&options, file_name, &data);

    if (res != cgltf_result_success) {
        std::cout << "Error: Could not parse GLB file, returning empty mesh data structure\n";
        cgltf_free(data);
        return new Mesh{};
    }
    res = cgltf_load_buffers(&options, data, file_name);
    if (res != cgltf_result_success) {
        std::cout << "Error: Could not load GLB file, returning empty mesh data structure\n";
        cgltf_free(data);
        return new Mesh{};
    }
    cgltf_mesh* mesh = data->meshes;
    cgltf_primitive* prim = &mesh->primitives[0];
    if (prim->type != cgltf_primitive_type_triangles) {
        std::cout << "Error: GLB primitives are not triangles, returning empty mesh data structure\n";
        cgltf_free(data);
        return new Mesh{};
    }
    
    cgltf_accessor* pos_accessor = nullptr;

    for (int i = 0; i < prim->attributes_count; i++) {
        if (prim->attributes[i].type == cgltf_attribute_type_position) {
            pos_accessor = prim->attributes[i].data;
            break;
        }
    }
    if (!pos_accessor) {
        std::cout << "Error: mesh has no positions, returning empty mesh data structure\n";
        cgltf_free(data);
        return new Mesh{};
    }
    Mesh* ret = new Mesh{};
    int vertex_count = pos_accessor->count;
    ret->vertices = new float[vertex_count * 9];
    float temp[3];
    for (int i = 0; i < vertex_count; i++) {
        cgltf_accessor_read_float(pos_accessor, i, temp, 3);
        ret->vertices[i * 3] = temp[0];
        ret->vertices[i * 3 + 1] = temp[1];
        ret->vertices[i * 3 + 2] = temp[2];
    }

    cgltf_accessor* index_accessor = prim->indices;
    ret->triangleCount = index_accessor->count;
    unsigned short index;
    ret->indices = new unsigned short[ret->triangleCount];
    if (prim->indices != NULL) {
        for (int i = 0; i < ret->triangleCount; i++) {
            index = cgltf_accessor_read_index(index_accessor, i);
            ret->indices[i] = index;
        }
    }

    cgltf_free(data);
    return ret;
}

// Mesh* LoadMesh(const char* file_name) {
//     CollisionMesh collision_mesh = LoadCollisionMesh(file_name);
//     Mesh* mesh = new Mesh{};
//     mesh->triangleCount = collision_mesh.triangle_count;
//     mesh->vertices = new float[mesh->triangleCount * 3];
//     memcpy(mesh->vertices, collision_mesh.vertices, mesh->triangleCount * 3 * sizeof(float));

//     mesh->indices = new unsigned short[mesh->triangleCount];
//     memcpy(mesh->indices, collision_mesh.indices, mesh->triangleCount * sizeof(unsigned short));
//     delete collision_mesh.indices;
//     delete collision_mesh.vertices;
//     return mesh;
// }

CollisionMesh LoadCollisionMesh(const char* file_name) {
    cgltf_options options{};
    cgltf_data* data = NULL;
    cgltf_result res = cgltf_parse_file(&options, file_name, &data);

    if (res != cgltf_result_success) {
        std::cout << "Error: Could not parse GLB file, returning empty mesh data structure\n";
        cgltf_free(data);
        return CollisionMesh{};
    }
    res = cgltf_load_buffers(&options, data, file_name);
    if (res != cgltf_result_success) {
        std::cout << "Error: Could not load GLB file, returning empty mesh data structure\n";
        cgltf_free(data);
        return CollisionMesh{};
    }
    cgltf_mesh* mesh = data->meshes;
    cgltf_primitive* prim = &mesh->primitives[0];
    if (prim->type != cgltf_primitive_type_triangles) {
        std::cout << "Error: GLB primitives are not triangles, returning empty mesh data structure\n";
        cgltf_free(data);
        return CollisionMesh{};
    }
    
    cgltf_accessor* pos_accessor = nullptr;

    for (int i = 0; i < prim->attributes_count; i++) {
        if (prim->attributes[i].type == cgltf_attribute_type_position) {
            pos_accessor = prim->attributes[i].data;
            break;
        }
    }
    if (!pos_accessor) {
        std::cout << "Error: mesh has no positions, returning empty mesh data structure\n";
        cgltf_free(data);
        return CollisionMesh{};
    }
    CollisionMesh cm{};
    int vertex_count = pos_accessor->count;
    cm.vertices = new float[vertex_count * 3];
    float temp[3];
    for (int i = 0; i < vertex_count; i++) {
        cgltf_accessor_read_float(pos_accessor, i, temp, 3);
        cm.vertices[i * 3] = temp[0];
        cm.vertices[i * 3 + 1] = temp[1];
        cm.vertices[i * 3 + 2] = temp[2];
    }

    cgltf_accessor* index_accessor = prim->indices;
    cm.triangle_count = index_accessor->count;
    unsigned short index;
    cm.indices = new unsigned short[cm.triangle_count];
    if (prim->indices != NULL) {
        for (int i = 0; i < cm.triangle_count; i++) {
            index = cgltf_accessor_read_index(index_accessor, i);
            cm.indices[i] = index;
        }
    }
    else {
        for (int i = 0; i < cm.triangle_count; i++) cm.indices[i] = i;
    }

    cgltf_free(data);
    return cm;
}
