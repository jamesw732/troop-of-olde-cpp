cmake_minimum_required(VERSION 3.10)
project(MyProject CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SHARED_DIR ${CMAKE_SOURCE_DIR}/src/shared)
set(CLIENT_DIR ${CMAKE_SOURCE_DIR}/src/client)
set(SERVER_DIR ${CMAKE_SOURCE_DIR}/src/server)
set(CLIENT_TEST_DIR ${CMAKE_SOURCE_DIR}/src/client_tests)

# Libraries
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

# Executables
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

# Tests
enable_testing()
# Create test library
add_library(client-test-lib INTERFACE ${CLIENT_TEST_DIR})
set_target_properties(client-test-lib PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_include_directories(client-test-lib INTERFACE ${CMAKE_SOURCE_DIR}/src)
# Create test_all executable, link with dependencies
add_executable(test_client src/test_client.cpp)
target_link_libraries(test_client PRIVATE
    raylib m pthread dl rt GL X11 flecs_static
    client-lib shared client-test-lib
)
add_test(NAME test_client COMMAND test_client)
set_target_properties(test_client PROPERTIES EXCLUDE_FROM_ALL TRUE)
# Create tests target for Makefile
add_custom_target(tests
    DEPENDS test_client
)

# Debug build
set(DEBUG_CXX_FLAGS -g -O0 -DDEBUG)
# Client debug build
add_executable(client_debug src/client.cpp)
target_compile_options(client_debug PRIVATE ${DEBUG_CXX_FLAGS})
target_link_libraries(client_debug PRIVATE
    raylib m pthread dl rt GL X11 flecs_static
    client-lib shared
)
set_target_properties(client_debug PROPERTIES EXCLUDE_FROM_ALL TRUE)
# Server debug build
add_executable(server_debug src/server.cpp)
target_compile_options(server_debug PRIVATE ${DEBUG_CXX_FLAGS})
target_link_libraries(server_debug PRIVATE
    raylib m pthread dl rt GL X11 flecs_static
    server-lib shared
)
set_target_properties(server_debug PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_custom_target(debug
    DEPENDS client_debug server_debug
)


# Copy fonts to the build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets/fonts DESTINATION ${CMAKE_BINARY_DIR}/assets)

# Optionally define a relative runtime path
add_compile_definitions(FONT_DIR="assets/fonts")
