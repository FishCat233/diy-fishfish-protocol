add_library(fishfish-socket STATIC
    socket.cpp
)

target_include_directories(fishfish-socket PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (WIN32)
    target_link_libraries(fishfish-socket PUBLIC
        ws2_32
    )
endif()
