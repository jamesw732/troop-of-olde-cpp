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

    bool operator!=(const Coordinate& other) {
        return !(x == other.x && y == other.y);
    }

    Coordinate operator-(const Coordinate& other) {
        return Coordinate{x - other.x, y - other.y};
    }
};

inline std::ostream& operator<<(std::ostream& os, const Coordinate coords) {
    std::cout << "(" << coords.x << ", " << coords.y << ")\n";
    return os;
}


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

using CoordsList = std::vector<Coordinate>;


template<typename T>
T random_choice(const std::vector<T>& choices, std::mt19937& gen);

Direction get_direction_from_diff(Coordinate diff);
std::array<Coordinate, 4> get_neighbors(Coordinate coords);
bool is_valid_coord(Coordinate coord, int rows, int cols);

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

template<typename T>
T random_choice(const std::vector<T>& choices, std::mt19937& gen) {
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

