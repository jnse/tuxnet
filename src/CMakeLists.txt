
add_library(tuxnet SHARED
    server.cpp
)

target_link_libraries(tuxnet "pthread" "ssl" "crypto")
