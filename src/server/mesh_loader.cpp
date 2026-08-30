#include <iostream>
#include <cstring>

#include "raylib.h"
#include "raymath.h"
#include "config.h"
#include "external/cgltf.h"
extern "C" {
#include "external/tinyobj_loader_c.h"
}

#include "mesh_loader.hpp"

#ifndef MAX_FILEPATH_LENGTH
    #define MAX_FILEPATH_LENGTH   4096      // Maximum length for filepaths (Linux PATH_MAX default value)
#endif
#if defined(_WIN32)
    #include <direct.h>     // Required for: _chdir() [Used in LoadOBJ()]
    #define CHDIR _chdir
#else
    #include <unistd.h>     // Required for: chdir() (POSIX) [Used in LoadOBJ()]
    #define CHDIR chdir
#endif

Model LoadServerGlb(const char* filename);
Model LoadServerObj(const char* filename);

Model LoadServerModel(const char* filename) {
    if (IsFileExtension(filename, ".gltf") || IsFileExtension(filename, ".glb")) return LoadServerGlb(filename);
    if (IsFileExtension(filename, ".obj")) return LoadServerObj(filename);
    return LoadModel(filename);
    /* if (IsFileExtension(filename, ".obj")) return LoadServerObj(filename); */
    /* return {}; */
}

static void ReleaseFileGLTFCallback(const struct cgltf_memory_options *memoryOptions, const struct cgltf_file_options *fileOptions, void *data)
{
    UnloadFileData((unsigned char*) data);
}

static cgltf_result LoadFileGLTFCallback(const struct cgltf_memory_options *memoryOptions, const struct cgltf_file_options *fileOptions, const char *path, cgltf_size *size, void **data)
{
    int filesize;
    unsigned char *filedata = LoadFileData(path, &filesize);

    if (filedata == NULL) return cgltf_result_io_error;

    *size = filesize;
    *data = filedata;

    return cgltf_result_success;
}

/*
    Modified copy of Raylib's LoadGLTF function. Loads only the data used by the server,
    which is just position and indices for collision checking.
*/
inline Model LoadServerGlb(const char* fileName)
{
    // Macro to simplify attributes loading code
    #define LOAD_ATTRIBUTE(accesor, numComp, srcType, dstPtr) LOAD_ATTRIBUTE_CAST(accesor, numComp, srcType, dstPtr, srcType)

    #define LOAD_ATTRIBUTE_CAST(accesor, numComp, srcType, dstPtr, dstType) \
    { \
        int n = 0; \
        srcType *buffer = (srcType *)accesor->buffer_view->buffer->data + accesor->buffer_view->offset/sizeof(srcType) + accesor->offset/sizeof(srcType); \
        for (unsigned int k = 0; k < accesor->count; k++) \
        {\
            for (int l = 0; l < numComp; l++) \
            {\
                dstPtr[numComp*k + l] = (dstType)buffer[n + l];\
            }\
            n += (int)(accesor->stride/sizeof(srcType));\
        }\
    }

    Model model = { 0 };

    // glTF file loading
    int dataSize = 0;
    unsigned char *fileData = LoadFileData(fileName, &dataSize);

    if (fileData == NULL) return model;

    // glTF data loading
    cgltf_options options = {};
    options.file.read = LoadFileGLTFCallback;
    options.file.release = ReleaseFileGLTFCallback;
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse(&options, fileData, dataSize, &data);

    if (result == cgltf_result_success)
    {
        if (data->file_type == cgltf_file_type_glb) TRACELOG(LOG_INFO, "MODEL: [%s] Model basic data (glb) loaded successfully", fileName);
        else if (data->file_type == cgltf_file_type_gltf) TRACELOG(LOG_INFO, "MODEL: [%s] Model basic data (glTF) loaded successfully", fileName);
        else TRACELOG(LOG_WARNING, "MODEL: [%s] Model format not recognized", fileName);

        TRACELOG(LOG_INFO, "    > Meshes count: %i", data->meshes_count);
        TRACELOG(LOG_DEBUG, "    > Buffers count: %i", data->buffers_count);

        // Force reading data buffers (fills buffer_view->buffer->data)
        // NOTE: If an uri is defined to base64 data or external path, it's automatically loaded
        result = cgltf_load_buffers(&options, data, fileName);
        if (result != cgltf_result_success) TRACELOG(LOG_INFO, "MODEL: [%s] Failed to load mesh/material buffers", fileName);

        int primitivesCount = 0;

        // NOTE: We will load every primitive in the glTF as a separate raylib Mesh
        // Determine total number of meshes needed from the node hierarchy
        for (unsigned int i = 0; i < data->nodes_count; i++)
        {
            cgltf_node *node = &(data->nodes[i]);
            cgltf_mesh *mesh = node->mesh;
            if (!mesh) continue;

            for (unsigned int p = 0; p < mesh->primitives_count; p++)
            {
                if (mesh->primitives[p].type == cgltf_primitive_type_triangles) primitivesCount++;
            }
        }
        TRACELOG(LOG_DEBUG, "    > Primitives (triangles only) count based on hierarchy : %i", primitivesCount);

        // Load our model data: meshes
        model.meshCount = primitivesCount;
        model.meshes = (Mesh *)RL_CALLOC(model.meshCount, sizeof(Mesh));

        //----------------------------------------------------------------------------------------------------

        // Load meshes data
        //
        // NOTE: Visit each node in the hierarchy and process any mesh linked from it
        //  - Each primitive within a glTF node becomes a raylib Mesh
        //  - The local-to-world transform of each node is used to transform the points/normals/tangents of the created Mesh(es)
        //  - Any glTF mesh linked from more than one Node (i.e. instancing) is turned into multiple Mesh's, as each Node will have its own transform applied
        //
        // WARNING: The code below disregards the scenes defined in the file, all nodes are used
        //----------------------------------------------------------------------------------------------------
        int meshIndex = 0;
        for (unsigned int i = 0; i < data->nodes_count; i++)
        {
            cgltf_node *node = &(data->nodes[i]);

            cgltf_mesh *mesh = node->mesh;
            if (!mesh) continue;

            cgltf_float worldTransform[16];
            cgltf_node_transform_world(node, worldTransform);

            Matrix worldMatrix = {
                worldTransform[0], worldTransform[4], worldTransform[8], worldTransform[12],
                worldTransform[1], worldTransform[5], worldTransform[9], worldTransform[13],
                worldTransform[2], worldTransform[6], worldTransform[10], worldTransform[14],
                worldTransform[3], worldTransform[7], worldTransform[11], worldTransform[15]
            };

            Matrix worldMatrixNormals = MatrixTranspose(MatrixInvert(worldMatrix));

            for (unsigned int p = 0; p < mesh->primitives_count; p++)
            {
                // NOTE: We only support primitives defined by triangles
                // Other alternatives: points, lines, line_strip, triangle_strip
                if (mesh->primitives[p].type != cgltf_primitive_type_triangles) continue;

                // NOTE: Attributes data could be provided in several data formats (8, 8u, 16u, 32...),
                // Only some formats for each attribute type are supported, read info at the top of this function!

                for (unsigned int j = 0; j < mesh->primitives[p].attributes_count; j++)
                {
                    // Check the different attributes for every primitive
                    if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_position)      // POSITION, vec3, float
                    {
                        cgltf_accessor *attribute = mesh->primitives[p].attributes[j].data;

                        // WARNING: SPECS: POSITION accessor MUST have its min and max properties defined

                        if (model.meshes[meshIndex].vertices != NULL) TRACELOG(LOG_WARNING, "MODEL: [%s] Vertices attribute data already loaded", fileName);
                        else
                        {
                            if ((attribute->type == cgltf_type_vec3) && (attribute->component_type == cgltf_component_type_r_32f))
                            {
                                // Init raylib mesh vertices to copy glTF attribute data
                                model.meshes[meshIndex].vertexCount = (int)attribute->count;
                                model.meshes[meshIndex].vertices = (float *)RL_MALLOC(attribute->count*3*sizeof(float));

                                // Load 3 components of float data type into mesh.vertices
                                LOAD_ATTRIBUTE(attribute, 3, float, model.meshes[meshIndex].vertices)

                                // Transform the vertices
                                float *vertices = model.meshes[meshIndex].vertices;
                                for (unsigned int k = 0; k < attribute->count; k++)
                                {
                                    Vector3 vt = Vector3Transform((Vector3){ vertices[3*k], vertices[3*k+1], vertices[3*k+2] }, worldMatrix);
                                    vertices[3*k] = vt.x;
                                    vertices[3*k+1] = vt.y;
                                    vertices[3*k+2] = vt.z;
                                }
                            }
                            else TRACELOG(LOG_WARNING, "MODEL: [%s] Vertices attribute data format not supported, use vec3 float", fileName);
                        }
                    }
                }

                // Load primitive indices data (if provided)
                if ((mesh->primitives[p].indices != NULL) && (mesh->primitives[p].indices->buffer_view != NULL))
                {
                    cgltf_accessor *attribute = mesh->primitives[p].indices;

                    model.meshes[meshIndex].triangleCount = (int)attribute->count/3;

                    if (model.meshes[meshIndex].indices != NULL) TRACELOG(LOG_WARNING, "MODEL: [%s] Indices attribute data already loaded", fileName);
                    else
                    {
                        if (attribute->component_type == cgltf_component_type_r_16u)
                        {
                            // Init raylib mesh indices to copy glTF attribute data
                            model.meshes[meshIndex].indices = (unsigned short *)RL_MALLOC(attribute->count*sizeof(unsigned short));

                            // Load unsigned short data type into mesh.indices
                            LOAD_ATTRIBUTE(attribute, 1, unsigned short, model.meshes[meshIndex].indices)
                        }
                        else if (attribute->component_type == cgltf_component_type_r_8u)
                        {
                            // Init raylib mesh indices to copy glTF attribute data
                            model.meshes[meshIndex].indices = (unsigned short *)RL_MALLOC(attribute->count*sizeof(unsigned short));
                            LOAD_ATTRIBUTE_CAST(attribute, 1, unsigned char, model.meshes[meshIndex].indices, unsigned short)

                        }
                        else if (attribute->component_type == cgltf_component_type_r_32u)
                        {
                            // Init raylib mesh indices to copy glTF attribute data
                            model.meshes[meshIndex].indices = (unsigned short *)RL_MALLOC(attribute->count*sizeof(unsigned short));
                            LOAD_ATTRIBUTE_CAST(attribute, 1, unsigned int, model.meshes[meshIndex].indices, unsigned short);

                            TRACELOG(LOG_WARNING, "MODEL: [%s] Indices data converted from u32 to u16, possible loss of data", fileName);
                        }
                        else TRACELOG(LOG_WARNING, "MODEL: [%s] Indices data format not supported, use u16", fileName);
                    }
                }
                else model.meshes[meshIndex].triangleCount = model.meshes[meshIndex].vertexCount/3;    // Unindexed mesh

                meshIndex++;       // Move to next mesh
            }
        }
        meshIndex = 0;

        // Free all cgltf loaded data
        cgltf_free(data);
    }
    else TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to load glTF data", fileName);

    // WARNING: cgltf requires the file pointer available while reading data
    UnloadFileData(fileData);

    return model;
}


inline Model LoadServerObj(const char *fileName)
{
    tinyobj_attrib_t objAttributes = { 0 };
    tinyobj_shape_t *objShapes = NULL;
    unsigned int objShapeCount = 0;

    tinyobj_material_t *objMaterials = NULL;
    unsigned int objMaterialCount = 0;

    Model model = { 0 };
    model.transform = MatrixIdentity();

    char *fileText = LoadFileText(fileName);

    if (fileText == NULL)
    {
        TRACELOG(LOG_WARNING, "MODEL: [%s] Unable to read obj file", fileName);
        return model;
    }

    char currentDir[MAX_FILEPATH_LENGTH] = { 0 };
    strncpy(currentDir, GetWorkingDirectory(), MAX_FILEPATH_LENGTH - 1); // Save current working directory
    const char *workingDir = GetDirectoryPath(fileName); // Switch to OBJ directory for material path correctness
    if (CHDIR(workingDir) != 0) TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to change working directory", workingDir);

    unsigned int dataSize = (unsigned int)strlen(fileText);

    unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;
    int ret = tinyobj_parse_obj(&objAttributes, &objShapes, &objShapeCount, &objMaterials, &objMaterialCount, fileText, dataSize, flags);

    if (ret != TINYOBJ_SUCCESS)
    {
        TRACELOG(LOG_WARNING, "MODEL: Unable to read obj data %s", fileName);
        return model;
    }

    UnloadFileText(fileText);

    unsigned int faceVertIndex = 0;
    unsigned int nextShape = 1;
    int lastMaterial = -1;
    unsigned int meshIndex = 0;

    // Count meshes
    unsigned int nextShapeEnd = objAttributes.num_face_num_verts;

    // See how many verts till the next shape
    if (objShapeCount > 1) nextShapeEnd = objShapes[nextShape].face_offset;

    // Walk all the faces
    for (unsigned int faceId = 0; faceId < objAttributes.num_faces; faceId++)
    {
        if (faceId >= nextShapeEnd)
        {
            // Try to find the last vert in the next shape
            nextShape++;
            if (nextShape < objShapeCount) nextShapeEnd = objShapes[nextShape].face_offset;
            else nextShapeEnd = objAttributes.num_face_num_verts; // This is actually the total number of face verts in the file, not faces
            meshIndex++;
        }
        else if ((lastMaterial != -1) && (objAttributes.material_ids[faceId] != lastMaterial))
        {
            meshIndex++; // If this is a new material, a new mesh is allocated
        }

        lastMaterial = objAttributes.material_ids[faceId];
        faceVertIndex += objAttributes.face_num_verts[faceId];
    }

    // Allocate the base meshes and materials
    model.meshCount = meshIndex + 1;
    model.meshes = (Mesh *)MemAlloc(sizeof(Mesh)*model.meshCount);

    if (objMaterialCount > 0)
    {
        model.materialCount = objMaterialCount;
        model.materials = (Material *)MemAlloc(sizeof(Material)*objMaterialCount);
    }
    else // Allocate at least one material
    {
        model.materialCount = 1;
        model.materials = (Material *)MemAlloc(sizeof(Material)*1);
    }

    model.meshMaterial = (int *)MemAlloc(sizeof(int)*model.meshCount);

    // See how many verts are in each mesh
    unsigned int *localMeshVertexCounts = (unsigned int *)MemAlloc(sizeof(unsigned int)*model.meshCount);

    faceVertIndex = 0;
    nextShapeEnd = objAttributes.num_face_num_verts;
    lastMaterial = -1;
    meshIndex = 0;
    unsigned int localMeshVertexCount = 0;

    nextShape = 1;
    if (objShapeCount > 1) nextShapeEnd = objShapes[nextShape].face_offset;

    // Walk all the faces
    for (unsigned int faceId = 0; faceId < objAttributes.num_faces; faceId++)
    {
        bool newMesh = false; // Is a new mesh required?
        if (faceId >= nextShapeEnd)
        {
            // Try to find the last vert in the next shape
            nextShape++;
            if (nextShape < objShapeCount) nextShapeEnd = objShapes[nextShape].face_offset;
            else nextShapeEnd = objAttributes.num_face_num_verts; // this is actually the total number of face verts in the file, not faces

            newMesh = true;
        }
        else if ((lastMaterial != -1) && (objAttributes.material_ids[faceId] != lastMaterial))
        {
            newMesh = true;
        }

        lastMaterial = objAttributes.material_ids[faceId];

        if (newMesh)
        {
            localMeshVertexCounts[meshIndex] = localMeshVertexCount;

            localMeshVertexCount = 0;
            meshIndex++;
        }

        faceVertIndex += objAttributes.face_num_verts[faceId];
        localMeshVertexCount += objAttributes.face_num_verts[faceId];
    }

    localMeshVertexCounts[meshIndex] = localMeshVertexCount;

    for (int i = 0; i < model.meshCount; i++)
    {
        // Allocate the buffers for each mesh
        unsigned int vertexCount = localMeshVertexCounts[i];

        model.meshes[i].vertexCount = vertexCount;
        model.meshes[i].triangleCount = vertexCount/3;

        model.meshes[i].vertices = (float *)MemAlloc(sizeof(float)*vertexCount*3);
        model.meshes[i].normals = (float *)MemAlloc(sizeof(float)*vertexCount*3);
    #if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
        model.meshes[i].texcoords = (float *)MemAlloc(sizeof(float)*vertexCount*2);
        model.meshes[i].colors = (unsigned char *)MemAlloc(sizeof(unsigned char)*vertexCount*4);
    #else
        if (objAttributes.texcoords != NULL && objAttributes.num_texcoords > 0)
            model.meshes[i].texcoords = (float *)MemAlloc(sizeof(float)*vertexCount*2);
        else model.meshes[i].texcoords = NULL;
        model.meshes[i].colors = NULL;
    #endif
    }

    MemFree(localMeshVertexCounts);
    localMeshVertexCounts = NULL;

    // Fill meshes
    faceVertIndex = 0;

    nextShapeEnd = objAttributes.num_face_num_verts;

    // See how many verts till the next shape
    nextShape = 1;
    if (objShapeCount > 1) nextShapeEnd = objShapes[nextShape].face_offset;
    lastMaterial = -1;
    meshIndex = 0;
    localMeshVertexCount = 0;

    // Walk all the faces
    for (unsigned int faceId = 0; faceId < objAttributes.num_faces; faceId++)
    {
        bool newMesh = false; // Is a new mesh required?
        if (faceId >= nextShapeEnd)
        {
            // Try to find the last vert in the next shape
            nextShape++;
            if (nextShape < objShapeCount) nextShapeEnd = objShapes[nextShape].face_offset;
            else nextShapeEnd = objAttributes.num_face_num_verts; // This is actually the total number of face verts in the file, not faces
            newMesh = true;
        }

        // If this is a new material, a new mesh is allocated
        if (lastMaterial != -1 && objAttributes.material_ids[faceId] != lastMaterial) newMesh = true;
        lastMaterial = objAttributes.material_ids[faceId];

        if (newMesh)
        {
            localMeshVertexCount = 0;
            meshIndex++;
        }

        int matId = 0;
        if ((lastMaterial >= 0) && (lastMaterial < (int)objMaterialCount)) matId = lastMaterial;

        model.meshMaterial[meshIndex] = matId;

        for (int f = 0; f < objAttributes.face_num_verts[faceId]; f++)
        {
            int vertIndex = objAttributes.faces[faceVertIndex].v_idx;
            int normalIndex = objAttributes.faces[faceVertIndex].vn_idx;
            int texcordIndex = objAttributes.faces[faceVertIndex].vt_idx;

            for (int i = 0; i < 3; i++) model.meshes[meshIndex].vertices[localMeshVertexCount*3 + i] = objAttributes.vertices[vertIndex*3 + i];

            if ((objAttributes.texcoords != NULL) && (texcordIndex != TINYOBJ_INVALID_INDEX) && (texcordIndex >= 0) && (model.meshes[meshIndex].texcoords))
            {
                for (int i = 0; i < 2; i++) model.meshes[meshIndex].texcoords[localMeshVertexCount*2 + i] = objAttributes.texcoords[texcordIndex*2 + i];
                model.meshes[meshIndex].texcoords[localMeshVertexCount*2 + 1] = 1.0f - model.meshes[meshIndex].texcoords[localMeshVertexCount*2 + 1];
            }

            if ((objAttributes.normals != NULL) && (normalIndex != TINYOBJ_INVALID_INDEX) && (normalIndex >= 0))
            {
                for (int i = 0; i < 3; i++) model.meshes[meshIndex].normals[localMeshVertexCount*3 + i] = objAttributes.normals[normalIndex*3 + i];
            }
            else
            {
                model.meshes[meshIndex].normals[localMeshVertexCount*3 + 0] = 0.0f;
                model.meshes[meshIndex].normals[localMeshVertexCount*3 + 1] = 1.0f;
                model.meshes[meshIndex].normals[localMeshVertexCount*3 + 2] = 0.0f;
            }
        #if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
            for (int i = 0; i < 4; i++) model.meshes[meshIndex].colors[localMeshVertexCount*4 + i] = 255;
        #endif
            faceVertIndex++;
            localMeshVertexCount++;
        }
    }

    tinyobj_attrib_free(&objAttributes);
    tinyobj_shapes_free(objShapes, objShapeCount);
    tinyobj_materials_free(objMaterials, objMaterialCount);

    // Restore current working directory
    if (CHDIR(currentDir) != 0)
    {
        TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to change working directory", currentDir);
    }

    return model;
}
