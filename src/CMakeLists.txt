
add_library(tuxnet SHARED
    server.cpp
    string.cpp
    log.cpp
    ip_address.cpp
)

target_link_libraries(tuxnet "pthread" "ssl" "crypto")
