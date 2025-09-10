cmake_minimum_required(VERSION 3.10)
project(MyProject CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(client src/client.cpp)
add_executable(server src/server.cpp)

target_link_libraries(client PRIVATE
    raylib m pthread dl rt GL X11 flecs_static
)
target_link_libraries(server PRIVATE
    raylib m pthread dl rt GL X11 flecs_static
)
