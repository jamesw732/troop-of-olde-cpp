#pragma once
#include "raylib.h"

#include "mapgen-util.hpp"


const int ROOM_SIZE = 64;

inline Image atlas = LoadImage(TEXTURE_DIR "rooms.png");

Rectangle get_atlas_rect(Direction direction);
Coordinate get_atlas_coords(Direction direction);

inline void create_image(Map map) {
    int x = map.cols;
    int y = map.rows;
    InitWindow(1, 1, "Map Generator");
    Image map_image = GenImageColor(x * ROOM_SIZE, y * ROOM_SIZE, BLACK);
    for (int row = 0; row < y; row++) {
        for (int col = 0; col < x; col++) {
            MapCell cell = map.get({col, row});
            if (cell.type == CellType::Vacant) {
                continue;
            }
            Direction direction = cell.direction;
            Rectangle src_rect = get_atlas_rect(direction);

            Rectangle dest_rect = {
                static_cast<float>(col * ROOM_SIZE),
                static_cast<float>(row * ROOM_SIZE),
                ROOM_SIZE,
                ROOM_SIZE
            };

            ImageDraw(
                &map_image,
                atlas,
                src_rect,
                dest_rect,
                WHITE
            );
            if (cell.type == CellType::Boss) {
                int font_size = 48;
                /* std::cout << "Found boss\n"; */
                int text_width = MeasureText("B", 48);
                float x_offset = (ROOM_SIZE - text_width) / 2.0;
                float y_offset = (ROOM_SIZE - font_size) / 2.0;
                ImageDrawText(
                    &map_image,
                    "B",
                    col * ROOM_SIZE + x_offset,
                    row * ROOM_SIZE + y_offset,
                    font_size,
                    RED
                );
            }
        }
    }
    ExportImage(map_image, "map.png");
    UnloadImage(map_image);
    CloseWindow();
}

inline Rectangle get_atlas_rect(Direction direction) {
    Coordinate atlas_coords = get_atlas_coords(direction);
    float start_x = atlas_coords.x * ROOM_SIZE;
    float start_y = atlas_coords.y * ROOM_SIZE;
    return Rectangle{start_x, start_y, ROOM_SIZE, ROOM_SIZE};
}

inline Coordinate get_atlas_coords(Direction direction) {
    switch((int) direction) {
        case (0b0010): return {0, 0}; break;
        case (0b0001): return {1, 0}; break;
        case (0b0100): return {2, 0}; break;
        case (0b1000): return {3, 0}; break;
        case (0b0011): return {4, 0}; break;
        case (0b0110): return {0, 1}; break;
        case (0b1010): return {1, 1}; break;
        case (0b0101): return {2, 1}; break;
        case (0b1001): return {3, 1}; break;
        case (0b1100): return {4, 1}; break;
        case (0b0111): return {0, 2}; break;
        case (0b1011): return {1, 2}; break;
        case (0b1110): return {2, 2}; break;
        case (0b1101): return {3, 2}; break;
        case (0b1111): return {4, 2}; break;
        default: return {-1, -1}; break;
    }
}
