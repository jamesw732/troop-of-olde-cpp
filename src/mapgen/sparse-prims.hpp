#include <algorithm>
#include <vector>

#include "mapgen-util.hpp"

bool has_charted_neighbor(Map map, Coordinate coords, Coordinate exclude);

inline Map sparse_prims(int x, int y) {
    Map map{x, y};

    // Generate coordinates on the map
    // This will be used to randomize boss location and gaps
    CoordsList coords;
    coords.reserve(x * y);
    for (int col = 0; col < x; col++) {
        for (int row = 0; row < y; row++) {
            coords.emplace_back(row, col);
        }
    }
    // Shuffle coordinates for randomized processing
    std::shuffle(coords.begin(), coords.end(), gen);

    // Choose boss location
    Coordinate boss_coords = coords[0];
    MapCell& boss_cell = map.get(boss_coords);
    boss_cell.type = CellType::Boss;
    coords.erase(coords.begin());

    // Choose gap count
    const int min_cellcount = std::max(3, (x - 1) * (y - 1));
    const int max_gapcount = x * y - min_cellcount;

    std::uniform_int_distribution<int> dist(0, max_gapcount);
    int gapcount = dist(gen);

    // Generate directions of paths
    std::vector<Coordinate> frontier {};

    // add boss first, force it to be one-way
    std::array<Coordinate, 4> boss_neighbors = get_neighbors(boss_coords);
    std::vector<Coordinate> viable_boss_neighbors {};
    for (Coordinate candidate_coords: boss_neighbors) {
        if (is_valid_coord(candidate_coords, map.rows, map.cols)
            && map.get(candidate_coords).type == CellType::None) {
            viable_boss_neighbors.push_back(candidate_coords);
        }
    }
    frontier.push_back(random_choice(viable_boss_neighbors, gen));
    map.get(boss_coords).direction = get_direction_from_diff(boss_coords - frontier[0]);

    bool first_cell = true;
    while (frontier.size() > 0) {
        Coordinate cur_coords = random_choice<Coordinate>(frontier, gen);
        std::array<Coordinate, 4> neighbors = get_neighbors(cur_coords);
        // Mark this cell as a room, and then choose its direction by randomly selecting the room it came from
        std::erase(frontier, cur_coords);
        map.get(cur_coords).type = CellType::Normal;
        std::vector<Coordinate> charted_neighbors{}; // The neighbors which are a cell we can extend from
        for (Coordinate coords: neighbors) {
            if (is_valid_coord(coords, map.rows, map.cols)
                && (map.get(coords).type == CellType::Normal
                || (first_cell && map.get(coords).type == CellType::Boss))) {
                charted_neighbors.push_back(coords);
            }
        }
        Coordinate from_neighbor = random_choice(charted_neighbors, gen);
        Coordinate diff = cur_coords - from_neighbor;
        map.get(cur_coords).direction |= get_direction_from_diff(diff);
        Coordinate other_diff = from_neighbor - cur_coords;
        map.get(from_neighbor).direction |= get_direction_from_diff(other_diff);

        // Add neighbors to frontier
        for (Coordinate neighbor_coords: neighbors) {
            if (is_valid_coord(neighbor_coords, map.rows, map.cols)
                && map.get(neighbor_coords).type == CellType::None
                && !has_charted_neighbor(map, neighbor_coords, cur_coords)) {
                frontier.push_back(neighbor_coords);
                map.get(neighbor_coords).type = CellType::Frontier;
            }
        }
        first_cell = false;
    }
    for (MapCell& cell: map.grid) {
        if (cell.type == CellType::None) {
            cell.type = CellType::Vacant;
        }
    }

    return map;
}

inline bool has_charted_neighbor(Map map, Coordinate coords, Coordinate exclude) {
    std::array<Coordinate, 4> neighbors = get_neighbors(coords);
    for (Coordinate neighbor_coords: neighbors) {
        CellType type = map.get(neighbor_coords).type ;
        if (is_valid_coord(neighbor_coords, map.rows, map.cols)
            && neighbor_coords != exclude
            && type == CellType::Normal
            || type == CellType::Boss
            || type == CellType::Frontier) {
            return true;
        }
    }
    return false;
}
