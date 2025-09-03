cmake_minimum_required(VERSION 3.10)
project(MyProject CXX)

add_executable(game src/main.cpp)

target_link_libraries(game PRIVATE raylib m pthread dl rt GL X11 flecs_static)
