cmake_minimum_required(VERSION 3.10)
project(MyProject CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SHARED_DIR ${CMAKE_SOURCE_DIR}/src/shared)
set(CLIENT_DIR ${CMAKE_SOURCE_DIR}/src/client)
set(SERVER_DIR ${CMAKE_SOURCE_DIR}/src/server)


# # Create shared interface library
add_library(shared INTERFACE ${SHARED_DIR})
target_include_directories(shared INTERFACE
    ${CMAKE_SOURCE_DIR}/src
)

# # Create client library
add_library(client-lib INTERFACE ${CLIENT_DIR})
target_include_directories(client-lib INTERFACE
    ${CMAKE_SOURCE_DIR}/src
)

# Create server library
add_library(server-lib INTERFACE ${SERVER_DIR})
target_include_directories(server-lib INTERFACE
    ${CMAKE_SOURCE_DIR}/src
)

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

# Create test_all executable, link with dependencies
add_executable(test_client src/test_client.cpp)
target_link_libraries(test_client PRIVATE
    raylib m pthread dl rt GL X11 flecs_static
    client-lib shared
)
