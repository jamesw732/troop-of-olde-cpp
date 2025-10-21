include(FetchContent)

# raylib-cpp
set(RAYLIB-CPP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/raylib-cpp")
if (EXISTS "${RAYLIB-CPP_DIR}/CMakeLists.txt")
    add_library(raylib-cpp INTERFACE)
    target_include_directories(raylib-cpp
        INTERFACE
        ${RAYLIB-CPP_DIR}/include
    )
else()
    FetchContent_Declare(
        raylib-cpp-fetch
        GIT_REPOSITORY https://github.com/RobLoach/raylib-cpp
        GIT_TAG v5.5.0
    )
    FetchContent_MakeAvailable(raylib-cpp-fetch)
    if (NOT TARGET raylib-cpp)
        add_library(raylib-cpp INTERFACE)
        target_include_directories(raylib-cpp
            INTERFACE
            ${raylib-cpp-fetch_SOURCE_DIR}/include
        )
    endif()
endif()

# ENet
set(ENET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/enet")
if (EXISTS "${ENET_DIR}/CMakeLists.txt")
    add_library(enet INTERFACE)
    target_include_directories(enet
        INTERFACE
        ${ENET_DIR}/include
    )
else()
    FetchContent_Declare(
        enet-fetch
        GIT_REPOSITORY https://github.com/zpl-c/enet.git
        GIT_TAG v2.6.5
    )
    FetchContent_MakeAvailable(enet-fetch)
    if (NOT TARGET enet)
        add_library(enet INTERFACE)
        target_include_directories(enet
            INTERFACE
            ${enet-fetch_SOURCE_DIR}/include
        )
    endif()
endif()

# Bitsery
set(BITSERY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bitsery")
if (EXISTS "${BITSERY_DIR}/CMakeLists.txt")
    add_library(bitsery INTERFACE)
    target_include_directories(bitsery
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/bitsery/include
    )
else()
    FetchContent_Declare(
        bitsery-fetch
        GIT_REPOSITORY https://github.com/fraillt/bitsery.git
        GIT_TAG v5.2.5
    )
    FetchContent_MakeAvailable(bitsery-fetch)
    if (NOT TARGET bitsery)
        add_library(bitsery INTERFACE)
        target_include_directories(bitsery
            INTERFACE
            ${bitsery-fetch_SOURCE_DIR}/include
        )
    endif()
endif()

set(DBG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dbg-macro")
if (EXISTS "${DBG_DIR}/CMakeLists.txt")
    add_library(dbg INTERFACE)
    target_include_directories(dbg INTERFACE ${DBG_DIR})
else()
    FetchContent_Declare(
        dbg-fetch
        GIT_REPOSITORY https://github.com/sharkdp/dbg-macro
        GIT_TAG v0.5.1
    )
    FetchContent_MakeAvailable(dbg-fetch)
    if (NOT TARGET dbg)
        add_library(dbg INTERFACE)
        target_include_directories(dbg
            INTERFACE
            ${dbg-fetch_SOURCE_DIR}
        )
    endif()
endif()

# raylib
set(RAYLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/raylib")
if (EXISTS "${RAYLIB_DIR}/CMakeLists.txt")
    add_subdirectory(${RAYLIB_DIR})
else()
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
    )
    FetchContent_MakeAvailable(raylib)
endif()

# flecs
set(FLECS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/flecs")
if (EXISTS "${FLECS_DIR}/CMakeLists.txt")
    add_subdirectory(${FLECS_DIR})
else()
    FetchContent_Declare(
        flecs
        GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
        GIT_TAG v4.1.1
    )
    FetchContent_MakeAvailable(flecs)
endif()

