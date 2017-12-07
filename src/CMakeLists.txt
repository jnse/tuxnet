
add_library(tuxnet SHARED
    server.cpp
    string.cpp
    log.cpp
)

target_link_libraries(tuxnet "pthread" "ssl" "crypto")
