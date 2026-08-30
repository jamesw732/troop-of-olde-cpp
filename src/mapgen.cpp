#include <cassert>

#include "mapgen/prims.hpp"
#include "mapgen/sparse-prims.hpp"
#include "mapgen/visualizer.hpp"

int main(int argc, char* argv[]) {
    int size = 8;
    if (argc > 1) {
        size = std::stoi(argv[1]);
    }
    Map map = sparse_prims(size, size);
    /* Map map = prims(size, size); */
    create_image(map);

    std::cout << map << "\n";

    /* for (auto& row: map.grid) { */
    /*     for (MapCell cell: row) { */
    /*         std::string character; */
    /*         if (cell.type == CellType::Boss) { */
    /*             character = "B"; */
    /*         } */
    /*         if (cell.type == CellType::Vacant) { */
    /*             character = "1"; */
    /*         } */
    /*         if (cell.type == CellType::Normal) { */
    /*             character = "0"; */
    /*         } */
    /*         std::cout << character; */
    /*     } */
    /*     std::cout << "\n"; */
    /* } */
    // Test nontrivial connectedness cases
    Map test_map(8, 8);
    test_map.get({0, 0}).type = CellType::Vacant;
    test_map.get({1, 1}).type = CellType::Vacant;
    assert((is_connected(test_map, 2, Coordinate{0, 1})));
    assert((!is_connected(test_map, 2, Coordinate{0, 2})));
    test_map.get({0, 2}).type = CellType::Vacant;
    assert((!is_connected(test_map, 2, Coordinate{0, 3})));
}
