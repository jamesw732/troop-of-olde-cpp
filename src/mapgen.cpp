#include <cassert>

#include "mapgen/mapgen.hpp"
#include "mapgen/visualizer.hpp"

int main() {
    /* Map<2, 2> map = prims<2, 2>(); */
    /* Map<8, 8> map = prims<8, 8>(); */
    constexpr int size = 8;
    Map<size, size> map = prims<size, size>();
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
    bool gaps[8][8] {};
    gaps[0][0] = 1;
    gaps[1][1] = 1;
    assert((is_connected<8, 8>(gaps, 2, Coordinate{0, 1})));
    assert((!is_connected<8, 8>(gaps, 2, Coordinate{0, 2})));
    gaps[0][2] = 1;
    assert((!is_connected<8, 8>(gaps, 2, Coordinate{0, 3})));
}
