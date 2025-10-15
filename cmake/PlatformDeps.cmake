function(link_platform_libs target)
    if (UNIX AND NOT APPLE)
        target_link_libraries(${target} PRIVATE
            m pthread dl rt GL X11
        )
    elseif (APPLE)
        target_link_libraries(${target} PRIVATE
            "-framework IOKit"
            "-framework Cocoa"
            "-framework OpenGL"
            "-framework CoreAudio"
            "-framework CoreVideo"
            "-framework AudioToolbox"
        )
    elseif (WIN32)
        target_link_libraries(${target} PRIVATE
            winmm gdi32 opengl32 user32 kernel32
        )
    endif()
endfunction()
