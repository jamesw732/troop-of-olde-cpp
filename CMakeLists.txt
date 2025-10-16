cmake_minimum_required(VERSION 3.10)
project(MyProject CXX)

include(cmake/PlatformDeps.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SHARED_DIR ${CMAKE_SOURCE_DIR}/src/shared)
set(CLIENT_DIR ${CMAKE_SOURCE_DIR}/src/client)
set(SERVER_DIR ${CMAKE_SOURCE_DIR}/src/server)
set(TEST_DIR ${CMAKE_SOURCE_DIR}/test)
set(CLIENT_TEST_DIR ${TEST_DIR}/client)
set(SERVER_TEST_DIR ${TEST_DIR}/server)

# External dependencies
add_subdirectory(deps)

add_custom_target(deps
    DEPENDS raylib flecs
)

# Libraries
# # Create shared interface library
add_library(shared INTERFACE)
target_include_directories(shared INTERFACE
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(shared INTERFACE dbg)

# # Create client library
add_library(client-lib INTERFACE)
target_include_directories(client-lib INTERFACE
    ${CMAKE_SOURCE_DIR}/src
)

# Create server library
add_library(server-lib INTERFACE)
target_include_directories(server-lib INTERFACE
    ${CMAKE_SOURCE_DIR}/src
)

# Executables
# Create client executable, link with dependencies
add_executable(client src/client.cpp)
target_link_libraries(client PRIVATE
    raylib flecs_static
    raylib-cpp enet bitsery
    client-lib shared
)
link_platform_libs(client)

# Create server executable, link with dependencies
add_executable(server src/server.cpp)
target_link_libraries(server PRIVATE
    raylib flecs_static
    raylib-cpp enet bitsery
    server-lib shared
)
link_platform_libs(server)

# Tests
enable_testing()
# Client tests
file(GLOB TEST_CLIENT_SOURCES CONFIGURE_DEPENDS "${CLIENT_TEST_DIR}/*.cpp")
add_executable(test_client ${TEST_CLIENT_SOURCES})
target_link_libraries(test_client PRIVATE
    raylib flecs_static
    raylib-cpp enet bitsery
    client-lib shared
)
link_platform_libs(test_client)
add_test(NAME test_client COMMAND test_client)
set_target_properties(test_client PROPERTIES EXCLUDE_FROM_ALL TRUE)
# Server tests
file(GLOB TEST_SERVER_SOURCES CONFIGURE_DEPENDS "${SERVER_TEST_DIR}/*.cpp")
add_executable(test_server ${TEST_SERVER_SOURCES})
target_link_libraries(test_server PRIVATE
    raylib flecs_static
    raylib-cpp enet bitsery
    client-lib shared
)
link_platform_libs(test_server)
add_test(NAME test_server COMMAND test_server)
set_target_properties(test_server PROPERTIES EXCLUDE_FROM_ALL TRUE)

# Create tests target for Makefile
add_custom_target(tests
    DEPENDS test_client test_server
)

# Debug build
set(DEBUG_CXX_FLAGS -g -O0 -DDEBUG)
# Client debug build
add_executable(client_debug src/client.cpp)
target_compile_options(client_debug PRIVATE ${DEBUG_CXX_FLAGS})
target_link_libraries(client_debug PRIVATE
    raylib flecs_static
    raylib-cpp enet bitsery
    client-lib shared
)
link_platform_libs(client_debug)
set_target_properties(client_debug PROPERTIES EXCLUDE_FROM_ALL TRUE)
# Server debug build
add_executable(server_debug src/server.cpp)
target_compile_options(server_debug PRIVATE ${DEBUG_CXX_FLAGS})
target_link_libraries(server_debug PRIVATE
    raylib flecs_static
    raylib-cpp enet bitsery
    server-lib shared
)
link_platform_libs(server_debug)
set_target_properties(server_debug PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_custom_target(debug
    DEPENDS client_debug server_debug
)


# Copy fonts to the build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets/fonts DESTINATION ${CMAKE_BINARY_DIR}/assets)

# Optionally define a relative runtime path
add_compile_definitions(FONT_DIR="assets/fonts")
