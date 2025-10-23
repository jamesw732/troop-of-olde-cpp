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

# Assets
# Fonts
file(COPY ${CMAKE_SOURCE_DIR}/assets/fonts DESTINATION ${CMAKE_BINARY_DIR}/assets)
add_compile_definitions(FONT_DIR="assets/fonts")

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

add_library(client-network STATIC ${CMAKE_SOURCE_DIR}/src/client/network.cpp)
target_include_directories(client-network PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(client-network PRIVATE enet flecs bitsery)

add_library(server-network STATIC ${CMAKE_SOURCE_DIR}/src/server/network.cpp)
target_include_directories(server-network PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(server-network PRIVATE enet flecs bitsery)

# Executables
# Create client executable, link with dependencies
add_executable(client src/client.cpp)
target_link_libraries(client PRIVATE
    raylib flecs_static
    raylib-cpp enet bitsery
    shared client-network
)
link_platform_libs(client)

# Create server executable, link with dependencies
add_executable(server src/server.cpp)
target_link_libraries(server PRIVATE
    raylib flecs_static
    raylib-cpp enet bitsery
    shared server-network
)
link_platform_libs(server)

# Debug build
set(DEBUG_CXX_FLAGS -g -O0 -DDEBUG)
# Debug libraries
add_library(client-network-debug STATIC ${CMAKE_SOURCE_DIR}/src/client/network.cpp)
target_include_directories(client-network-debug PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_options(client-network-debug PRIVATE ${DEBUG_CXX_FLAGS})
target_link_libraries(client-network-debug PRIVATE enet flecs bitsery dbg)
set_target_properties(client-network-debug PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_library(server-network-debug STATIC ${CMAKE_SOURCE_DIR}/src/server/network.cpp)
target_include_directories(server-network-debug PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_options(server-network-debug PRIVATE ${DEBUG_CXX_FLAGS})
target_link_libraries(server-network-debug PRIVATE enet flecs bitsery dbg)
set_target_properties(server-network-debug PROPERTIES EXCLUDE_FROM_ALL TRUE)

# Client debug build
add_executable(client_debug src/client.cpp)
target_compile_options(client_debug PRIVATE ${DEBUG_CXX_FLAGS})
target_link_libraries(client_debug PRIVATE
    raylib flecs_static
    raylib-cpp enet bitsery dbg
    shared client-network-debug
)
link_platform_libs(client_debug)
set_target_properties(client_debug PROPERTIES EXCLUDE_FROM_ALL TRUE)
# Server debug build
add_executable(server_debug src/server.cpp)
target_compile_options(server_debug PRIVATE ${DEBUG_CXX_FLAGS})
target_link_libraries(server_debug PRIVATE
    raylib flecs_static
    raylib-cpp enet bitsery dbg
    shared server-network-debug
)
link_platform_libs(server_debug)
set_target_properties(server_debug PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_custom_target(debug
    DEPENDS client_debug server_debug
)

# Tests
enable_testing()
set(TEST_TARGETS "")
# Client tests
file(GLOB TEST_FILES "${CLIENT_TEST_DIR}/*.cpp")
foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    set(TEST_NAME "client.${TEST_NAME}")
    add_executable(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_NAME}
        raylib flecs_static
        raylib-cpp enet bitsery
        shared client-network-debug
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_target_properties(${TEST_NAME} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    target_compile_options(${TEST_NAME} PRIVATE ${DEBUG_CXX_FLAGS})

    list(APPEND TEST_TARGETS ${TEST_NAME})
endforeach()
# Server tests
file(GLOB TEST_FILES "${SERVER_TEST_DIR}/*.cpp")
foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    set(TEST_NAME "server.${TEST_NAME}")
    add_executable(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_NAME}
        raylib flecs_static
        raylib-cpp enet bitsery
        shared server-network-debug
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_target_properties(${TEST_NAME} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    target_compile_options(${TEST_NAME} PRIVATE ${DEBUG_CXX_FLAGS})

    list(APPEND TEST_TARGETS ${TEST_NAME})
endforeach()
# Create tests target for Makefile
add_custom_target(tests
    DEPENDS ${TEST_TARGETS}
)

