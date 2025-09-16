cmake_minimum_required(VERSION 3.10)
project(MyProject CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SHARED_DIR ${CMAKE_SOURCE_DIR}/src/shared)
set(CLIENT_DIR ${CMAKE_SOURCE_DIR}/src/client)
set(SERVER_DIR ${CMAKE_SOURCE_DIR}/src/server)


# Create shared interface library
add_library(shared
    ${SHARED_DIR}/util.cpp
)
set_target_properties(shared PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(shared PUBLIC ${CMAKE_SOURCE_DIR}/src)


# Create client library
add_library(client-lib
    ${CLIENT_DIR}/systems/movement_system.cpp
    ${CLIENT_DIR}/systems/render_system.cpp
)
target_link_libraries(client-lib PUBLIC shared)
target_include_directories(client-lib PUBLIC ${CMAKE_SOURCE_DIR}/src)


# Create server library
add_library(server-lib
    ${SERVER_DIR}/systems/movement_system.cpp
)
target_link_libraries(server-lib PUBLIC shared)
target_include_directories(server-lib PUBLIC ${CMAKE_SOURCE_DIR}/src)

# Create client executable, link with dependencies
add_executable(client src/client.cpp)
target_link_libraries(client PRIVATE
    raylib m pthread dl rt GL X11 flecs_static
    client-lib shared
)


# Create server executable, link with dependencies
add_executable(server src/server.cpp)
target_link_libraries(server PRIVATE
    raylib m pthread dl rt GL X11 flecs_static
    server-lib shared
)
