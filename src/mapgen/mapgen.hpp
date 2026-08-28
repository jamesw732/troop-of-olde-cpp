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

struct Map {
    int rows;
    int cols;
    std::vector<MapCell> grid;

    Map(int x, int y) : cols(x), rows(y), grid(rows * cols) {
    }

    MapCell& get(Coordinate coords) {
        return grid[cols * coords.y + coords.x];
    }

    friend std::ostream& operator<<(std::ostream& os, Map& map) {
         for (int row = 0; row < map.rows; ++row) {
            for (int col = 0; col < map.cols; ++col) {
                std::cout << map.get({col, row}).type;
            }
            os << "\n";
        }
         os << "\n";
         for (int row = 0; row < map.rows; ++row) {
            for (int col = 0; col < map.cols; ++col) {
                std::cout << map.get({col, row}).direction;
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

bool is_valid_gap(Map map, Coordinate coord, uint32_t gapcount = -1, Coordinate root = {0, 0});
bool is_connected(Map map, uint32_t gapcount = -1, Coordinate root = {0, 0});
std::array<Coordinate, 4> get_neighbors(Coordinate coords);
bool is_valid_coord(Coordinate coord, int rows, int cols);


inline Map prims(int x, int y) {
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

    // Choose gap locations
    CoordsList gap_coords {};
    gap_coords.reserve(gapcount);
    int cur_gapcount = 0;
    for (int cur_gapcount = 0; cur_gapcount < gapcount; cur_gapcount++) {
        int cur_idx = 1;
        while (true) {
            Coordinate cur_coords = coords[cur_idx];
            if (is_valid_gap(map, cur_coords, cur_gapcount + 1, boss_coords)) {
                gap_coords.emplace_back(cur_coords);
                map.get(cur_coords).type = CellType::Vacant;
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
        if (is_valid_coord(candidate_coords, map.rows, map.cols)
            && map.get(candidate_coords).type == CellType::None) {
            viable_boss_neighbors.push_back(candidate_coords);
        }
    }
    frontier.push_back(random_choice(viable_boss_neighbors, gen));
    map.get(boss_coords).direction = get_direction_from_diff(boss_coords - frontier[0]);

    bool first_cell = true;
    for (int i = 0; i < (x * y) - gapcount - 1; i++) {
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
        for (Coordinate candidate_coords: neighbors) {
            if (is_valid_coord(candidate_coords, map.rows, map.cols)
                && map.get(candidate_coords).type == CellType::None) {
                frontier.push_back(candidate_coords);
                map.get(candidate_coords).type = CellType::Frontier;
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

inline bool is_valid_gap(Map map, Coordinate coord, uint32_t gapcount, Coordinate root) {
    map.get(coord).type = CellType::Vacant;
    bool ret = is_connected(map, gapcount, root);
    map.get(coord).type = CellType::None;
    return ret;
}

inline bool is_connected(Map map, uint32_t gapcount, Coordinate root) {
    uint32_t visited_ct = 0;
    std::vector<bool> visited(map.cols * map.rows);
    CoordsList frontier{};
    // Start at the root, which is boss
    frontier.push_back(root);
    visited[root.y * map.cols + root.x] = 1;
    // process frontier elements and add their unvisited neighbors to the visited grid
    bool first_run = true;
    while (frontier.size() > 0) {
        Coordinate cur_coords = frontier.back();
        frontier.pop_back();
        
        std::array<Coordinate, 4> neighbors = get_neighbors(cur_coords);
        for (Coordinate coord: neighbors) {
            if (is_valid_coord(coord, map.rows, map.cols)
                && !visited[coord.y * map.cols + coord.x]
                && map.get(coord).type == CellType::None) {
                frontier.push_back(coord);
                visited[coord.y * map.cols + coord.x] = 1;
                visited_ct++;
                // Connectedness needs to honor one-way boss, only add at most one neighbor
                if (first_run) {
                    first_run = false;
                    break;
                }
            }
        }
    }
    return gapcount == map.rows * map.cols - visited_ct - 1; // bfs should have reached every room but gaps and boss
}

inline std::array<Coordinate, 4> get_neighbors(Coordinate coords) {
    return {{
        {coords.x - 1, coords.y},
        {coords.x + 1, coords.y},
        {coords.x, coords.y - 1},
        {coords.x, coords.y + 1}
    }};
}

inline bool is_valid_coord(Coordinate coord, int rows, int cols) {
    return coord.y >= 0
        && coord.x >= 0
        && coord.y < rows
        && coord.x < cols;
}
