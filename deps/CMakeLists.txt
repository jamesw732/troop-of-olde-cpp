# ENet
set(ENET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/enet")
add_library(enet INTERFACE)
target_include_directories(enet
    INTERFACE
    ${ENET_DIR}/include
)
set_target_properties(enet PROPERTIES
    LINKER_LANGUAGE C
    LANGUAGE C
)

# Bitsery
set(BITSERY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bitsery")
add_library(bitsery INTERFACE)
target_include_directories(bitsery
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/bitsery/include
)

# Dbg
set(DBG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dbg-macro")
add_library(dbg INTERFACE)
target_include_directories(dbg INTERFACE ${DBG_DIR})

# raylib
set(RAYLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/raylib")
# add_library(raylib STATIC IMPORTED)
# set_target_properties(raylib PROPERTIES
#     IMPORTED_LOCATION
#     "${RAYLIB_DIR}/librarylib.a"

#     INTERFACE_INCLUDE_DIRECTORIES
#     "${RAYLIB_DIr}/include"
# )
add_subdirectory(${RAYLIB_DIR})

# flecs
set(FLECS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/flecs")
add_library(flecs STATIC IMPORTED)
# set_target_properties(flecs PROPERTIES
#     IMPORTED_LOCATION
#     "${FLECS_DIR}/libflecs_static.a"

#     INTERFACE_INCLUDE_DIRECTORIES
#     "${FLECS_DIR}/include"
# )
add_subdirectory(${FLECS_DIR})

