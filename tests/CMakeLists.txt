link_directories("${CMAKE_BINARY_DIR}")

add_executable(server server/server.cpp)
target_link_libraries(server tuxnet)

