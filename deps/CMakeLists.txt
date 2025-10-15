add_library(raylib-cpp INTERFACE)
target_include_directories(raylib-cpp
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/raylib-cpp/include
)

add_library(enet INTERFACE)
target_include_directories(enet
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/enet/include
)

add_library(bitsery INTERFACE)
target_include_directories(bitsery
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/bitsery/include
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/raylib)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/flecs)
