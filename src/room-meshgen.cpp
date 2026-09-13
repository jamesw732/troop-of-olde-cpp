#include "mapgen/mesh-generator.hpp"

int main() {
    /* ExportRoomMtl("room.mtl"); */
    /* Mesh floor = CreateTestTriangle({0, 0, 0}, {1, 0, 0}, {0, 0, 1}); */
    /* Mesh interior = CreateTestTriangle({0, 0, 0}, {1, 0, 0}, {0, 1, 0}); */
    /* Mesh exterior = CreateTestTriangle({0, 0, 0}, {0, 1, 0}, {0, 0, 1}); */
    std::vector<Mesh> meshes = generate_room_meshes((Direction) 11);
    ExportRoomObj(
        "test-room.obj",
        "room.mtl",
        meshes[0],
        meshes[1],
        meshes[2]
    );

    for (int i = 1; i < 16; i++) {
        Direction directions = (Direction) i;
        std::string filename = "room_" + std::to_string(i) + ".obj";
        std::vector<Mesh> meshes = generate_room_meshes((Direction) directions);
        ExportRoomObj(
            filename.c_str(),
            "room.mtl",
            meshes[0],
            meshes[1],
            meshes[2]
        );
    }
}
