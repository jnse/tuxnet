cmake_minimum_required(VERSION 2.8.10)
project(tuxnet)

# Enable C++11 support.
set (CMAKE_CXX_FLAGS "--std=gnu++11 ${CMAKE_CXX_FLAGS}")

include_directories("${CMAKE_SOURCE_DIR}/include/") 

add_subdirectory(src)
add_subdirectory(tests)

enable_testing()
add_test(SERVER tests/server)

