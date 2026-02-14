cmake_minimum_required(VERSION 3.10)
project(MyProject CXX)

include(cmake/PlatformDeps.cmake)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
    add_compile_definitions(DBG_MACRO_NO_WARNING)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(SHARED_DIR ${SRC_DIR}/shared)
set(CLIENT_DIR ${SRC_DIR}/client)
set(SERVER_DIR ${SRC_DIR}/server)
set(TEST_DIR ${CMAKE_SOURCE_DIR}/test)
set(CLIENT_TEST_DIR ${TEST_DIR}/client)
set(SERVER_TEST_DIR ${TEST_DIR}/server)


# External dependencies
set(FLECS_SHARED OFF CACHE BOOL
    "Do not build shared Flecs library" FORCE)
add_subdirectory(deps)

# Copy asset directories to build path
file(COPY ${CMAKE_SOURCE_DIR}/assets/fonts DESTINATION ${CMAKE_BINARY_DIR}/assets)
file(COPY ${CMAKE_SOURCE_DIR}/assets/models DESTINATION ${CMAKE_BINARY_DIR}/assets)


# PCH
add_library(pch "${SHARED_DIR}/pch.cpp")
target_link_libraries(pch PRIVATE
    raylib flecs_static bitsery dbg
)
target_precompile_headers(pch PRIVATE "${SHARED_DIR}/pch.hpp")

add_library(pch-networking "${SHARED_DIR}/pch-networking.cpp")
target_link_libraries(pch-networking PRIVATE
    flecs_static enet bitsery dbg
)
target_precompile_headers(pch-networking PRIVATE "${SHARED_DIR}/pch-networking.hpp")

# Client network static library
add_library(client-network STATIC ${CMAKE_SOURCE_DIR}/src/client/network.cpp)
target_link_libraries(client-network PRIVATE
    dbg enet flecs_static bitsery
)
target_precompile_headers(client-network REUSE_FROM pch-networking)

# Server network static library
add_library(server-network STATIC ${CMAKE_SOURCE_DIR}/src/server/network.cpp)
target_link_libraries(server-network PRIVATE
    enet flecs_static bitsery dbg
)
target_precompile_headers(server-network REUSE_FROM pch-networking)

# Server mesh loader
add_library(server-meshloader STATIC ${CMAKE_SOURCE_DIR}/src/server/mesh_loader.cpp)
target_link_libraries(server-meshloader PRIVATE raylib)

# Executables
# Create client executable, link with dependencies
add_executable(client src/client.cpp)
target_link_libraries(client PRIVATE
    raylib flecs_static
    bitsery dbg
    client-network
)
link_platform_libs(client)
target_precompile_headers(client REUSE_FROM pch)
target_compile_definitions(client PRIVATE FONT_DIR="assets/fonts/" MODEL_DIR="assets/models/")

# Create server executable, link with dependencies
add_executable(server src/server.cpp)
target_link_libraries(server PRIVATE
    raylib flecs_static
    bitsery dbg
    server-network server-meshloader
)
link_platform_libs(server)
target_precompile_headers(server REUSE_FROM pch)
# Assets
file(COPY ${CMAKE_SOURCE_DIR}/assets/models DESTINATION ${CMAKE_BINARY_DIR}/assets)
target_compile_definitions(server PRIVATE MODEL_DIR="assets/models/")

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
        bitsery
        client-network
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_target_properties(${TEST_NAME} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    target_include_directories(${TEST_NAME} PRIVATE ${SRC_DIR})

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
        bitsery
        server-network server-meshloader
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_target_properties(${TEST_NAME} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    target_include_directories(${TEST_NAME} PRIVATE ${SRC_DIR})

    list(APPEND TEST_TARGETS ${TEST_NAME})
endforeach()
# Create tests target for Makefile
add_custom_target(tests
    DEPENDS ${TEST_TARGETS}
)

add_executable(logger src/logger.cpp)
target_link_libraries(logger PRIVATE
    raylib flecs_static
    bitsery dbg
)
link_platform_libs(logger)
