#include "mapgen/mesh-generator.hpp"

int main() {
    for (int i = 1; i < 16; i++) {
        Direction directions = (Direction) i;
        Mesh mesh = generate_room_mesh(directions);
        std::string filename = "room_" + std::to_string(i) + ".obj";
        ExportMesh(mesh, filename.c_str());
    }
}
