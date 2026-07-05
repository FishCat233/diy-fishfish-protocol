add_executable(fishfish-server
    main.cpp
    server.cpp
    connection.cpp
    protocol.cpp
)

target_link_libraries(fishfish-server PRIVATE
    fishfish-socket
    # protobuf::libprotobuf
)
