
add_library(tuxnet SHARED
    server.cpp
    string.cpp
    log.cpp
    ip_address.cpp
    socket_address.cpp
    socket.cpp
)

target_link_libraries(tuxnet "pthread" "ssl" "crypto")
