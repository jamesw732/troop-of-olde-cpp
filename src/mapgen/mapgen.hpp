#pragma once
#include <array>
#include <iostream>
#include <cstdint>
#include <random>

inline std::random_device rd;
inline std::mt19937 gen(rd());


enum class Direction : uint8_t {
    None = 0,
    Right = 1 << 0,
    Up = 1 << 1,
    Down = 1 << 2,
    Left = 1 << 3,
};

constexpr Direction operator|(Direction a, Direction b)
{
    return static_cast<Direction>(
        static_cast<uint8_t>(a) | static_cast<uint8_t>(b)
    );
}

constexpr Direction& operator|=(Direction& a, Direction b) {
    a = a | b;
    return a;
}

constexpr Direction operator&(Direction a, Direction b)
{
    return static_cast<Direction>(
        static_cast<uint8_t>(a) & static_cast<uint8_t>(b)
    );
}


enum class CellType : uint8_t {
    None,
    Frontier,
    Vacant,
    Normal,
    Base,
    Boss
};

inline std::ostream& operator<<(std::ostream& os, const CellType cell_type) {
    switch (cell_type) {
        case CellType::Vacant: std::cout << "."; break;
        case CellType::Normal: std::cout << "0"; break;
        case CellType::Base: std::cout << "@"; break;
        case CellType::Boss: std::cout << "B"; break;
        default: std::cout << "?"; break;
    }
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Direction direction) {
    /* for (int i = 3; i >= 0; --i) { */
    /*     os << (((uint8_t) direction >> i) & 1); */
    /* } */
    std::cout << std::hex << (int) direction;
    return os;
}

struct MapCell {
    CellType type = CellType::None;
    Direction direction = Direction::None;
};

struct Coordinate {
    int x;
    int y;

    bool operator==(const Coordinate& other) {
        return x == other.x && y == other.y;
    }

    Coordinate operator-(const Coordinate& other) {
        return Coordinate{x - other.x, y - other.y};
    }
};

template <int x, int y>
struct Map {
    MapCell grid[y][x] = {};

    friend std::ostream& operator<<(std::ostream& os, const Map& map) {
         for (int row = 0; row < y; ++row) {
            for (int col = 0; col < x; ++col) {
                std::cout << map.grid[row][col].type;
            }
            os << "\n";
        }
         os << "\n";
         for (int row = 0; row < y; ++row) {
            for (int col = 0; col < x; ++col) {
                std::cout << map.grid[row][col].direction;
                std::cout << " ";
            }
            os << "\n";
        }
        return os;
    }

};

//  PRIM'S ALGORITHM
using CoordsList = std::vector<Coordinate>;

template<typename T>
T random_choice(std::vector<T> choices, std::mt19937 gen);

Direction get_direction_from_diff(Coordinate diff);

template<int x, int y>
bool is_valid_gap(bool grid[y][x], Coordinate coord, uint32_t gapcount = -1, Coordinate root = {0, 0});

template<int x, int y>
bool is_connected(bool grid[y][x], uint32_t gapcount = -1, Coordinate root = {0, 0});

std::array<Coordinate, 4> get_neighbors(Coordinate coords);

template<int x, int y>
bool is_valid_coord(Coordinate coord);


template <int x, int y>
Map<x, y> prims() {
    Map<x, y> map{};

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
    MapCell& boss_cell = map.grid[boss_coords.y][boss_coords.x];
    boss_cell.type = CellType::Boss;
    coords.erase(coords.begin());

    // Choose gap count
    constexpr int min_cellcount = std::max(3, (x - 1) * (y - 1));
    constexpr int max_gapcount = x * y - min_cellcount;

    std::uniform_int_distribution<int> dist(0, max_gapcount);
    int gapcount = dist(gen);

    // Choose gap locations
    CoordsList gap_coords {};
    bool gap_grid[y][x] {0};
    gap_coords.reserve(gapcount);
    int cur_gapcount = 0;
    for (int cur_gapcount = 0; cur_gapcount < gapcount; cur_gapcount++) {
        int cur_idx = 1;
        while (true) {
            Coordinate cur_coords = coords[cur_idx];
            if (is_valid_gap<x, y>(gap_grid, cur_coords, cur_gapcount + 1, boss_coords)) {
                gap_coords.emplace_back(cur_coords);
                gap_grid[cur_coords.y][cur_coords.x] = 1;
                map.grid[cur_coords.y][cur_coords.x].type = CellType::Vacant;
                coords.erase(coords.begin() + cur_idx);
                break;
            }
            cur_idx++;
        }
    }

    // Generate directions of paths
    std::vector<Coordinate> frontier {};

    // add boss first, force it to be one-way
    std::array<Coordinate, 4> boss_neighbors = get_neighbors(boss_coords);
    std::vector<Coordinate> viable_boss_neighbors {};
    for (Coordinate candidate_coords: boss_neighbors) {
        if (is_valid_coord<x, y>(candidate_coords)
            && map.grid[candidate_coords.y][candidate_coords.x].type == CellType::None) {
            viable_boss_neighbors.push_back(candidate_coords);
        }
    }
    frontier.push_back(random_choice(viable_boss_neighbors, gen));
    map.grid[boss_coords.y][boss_coords.x].direction = get_direction_from_diff(boss_coords - frontier[0]);

    bool first_cell = true;
    for (int i = 0; i < (x * y) - gapcount - 1; i++) {
        Coordinate cur_coords = random_choice<Coordinate>(frontier, gen);
        std::array<Coordinate, 4> neighbors = get_neighbors(cur_coords);
        // Mark this cell as a room, and then choose its direction by randomly selecting the room it came from
        std::erase(frontier, cur_coords);
        map.grid[cur_coords.y][cur_coords.x].type = CellType::Normal;
        std::vector<Coordinate> charted_neighbors{}; // The neighbors which are a cell we can extend from
        for (Coordinate coords: neighbors) {
            if (is_valid_coord<x, y>(coords)
                && (map.grid[coords.y][coords.x].type == CellType::Normal
                || (first_cell && map.grid[coords.y][coords.x].type == CellType::Boss))) {
                charted_neighbors.push_back(coords);
            }
        }
        Coordinate from_neighbor = random_choice(charted_neighbors, gen);
        Coordinate diff = cur_coords - from_neighbor;
        map.grid[cur_coords.y][cur_coords.x].direction |= get_direction_from_diff(diff);
        Coordinate other_diff = from_neighbor - cur_coords;
        map.grid[from_neighbor.y][from_neighbor.x].direction |= get_direction_from_diff(other_diff);

        // Add neighbors to frontier
        for (Coordinate candidate_coords: neighbors) {
            if (is_valid_coord<x, y>(candidate_coords)
                && map.grid[candidate_coords.y][candidate_coords.x].type == CellType::None) {
                frontier.push_back(candidate_coords);
                map.grid[candidate_coords.y][candidate_coords.x].type = CellType::Frontier;
            }
        }
        first_cell = false;
    }

    return map;
}

template<typename T>
T random_choice(std::vector<T> choices, std::mt19937 gen) {
    std::uniform_int_distribution<std::size_t> dist(0, choices.size() - 1);
    return choices[dist(gen)];
}

inline Direction get_direction_from_diff(Coordinate diff) {
    if (diff.y == 1) {
        return Direction::Up;
    }
    if (diff.y == -1) {
        return Direction::Down;
    }
    if (diff.x == 1) {
        return Direction::Left;
    }
    if (diff.x == -1) {
        return Direction::Right;
    }
    return Direction::None;
}

/**
 * grid: 2D array where each index represents whether the cell is vacant or not
 * coord: coordinates of a candidate cell we want to make vacant. If it would make the grid unconnected, reject
 */
template<int x, int y>
bool is_valid_gap(bool grid[y][x], Coordinate coord, uint32_t gapcount, Coordinate root) {
    grid[coord.y][coord.x] = 1;
    bool ret = is_connected<x, y>(grid, gapcount, root);
    grid[coord.y][coord.x] = 0;
    return ret;
}

template<int x, int y>
bool is_connected(bool gaps[y][x], uint32_t gapcount, Coordinate root) {
    uint32_t visited_ct = 0;
    bool visited[y][x] {0}; // cells which have been in the frontier
    CoordsList frontier{};
    // Start at the root, which is boos
    frontier.push_back(root);
    visited[root.y][root.x] = 1;
    // process frontier elements and add their unvisited neighbors to the visited grid
    bool first_run = true;
    while (frontier.size() > 0) {
        Coordinate cur_coords = frontier.back();
        frontier.pop_back();
        
        std::array<Coordinate, 4> neighbors = get_neighbors(cur_coords);
        for (Coordinate coord: neighbors) {
            if (is_valid_coord<x, y>(coord)
                && !visited[coord.y][coord.x]
                && !gaps[coord.y][coord.x]) {
                frontier.push_back(coord);
                visited[coord.y][coord.x] = 1;
                visited_ct++;
                // Connectedness needs to honor one-way boss, only add at most one neighbor
                if (first_run) {
                    first_run = false;
                    break;
                }
            }
        }
    }
    return gapcount == x * y - visited_ct - 1; // bfs should have reached every room but gaps and boss
}

inline std::array<Coordinate, 4> get_neighbors(Coordinate coords) {
    return {{
        {coords.x - 1, coords.y},
        {coords.x + 1, coords.y},
        {coords.x, coords.y - 1},
        {coords.x, coords.y + 1}
    }};
}

template<int x, int y>
bool is_valid_coord(Coordinate coord) {
    return coord.y >= 0
        && coord.x >= 0
        && coord.y < y
        && coord.x < x;
}
