
add_library(tuxnet SHARED
    server.cpp
    string.cpp
    protocol.cpp
    config.cpp
    log.cpp
    ip_address.cpp
    socket_address.cpp
    event.cpp
    peer.cpp
    socket.cpp
)

target_link_libraries(tuxnet pthread)
