function(link_platform_libs target)
    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        target_link_libraries(${target} PRIVATE
            winmm gdi32 opengl32 user32 kernel32
        )
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_link_libraries(${target} PRIVATE
            m pthread dl rt GL X11
        )
    endif()
endfunction()
