add_library(raylib-cpp INTERFACE)
target_include_directories(raylib-cpp
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/raylib-cpp
)

add_library(enet INTERFACE)
target_include_directories(enet
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/enet
)

add_library(bitsery INTERFACE)
target_include_directories(bitsery
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/bitsery
)
