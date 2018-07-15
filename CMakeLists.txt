cmake_minimum_required(VERSION 2.8.10)
project(tuxnet)

# Enable C++17 support.
set (CMAKE_CXX_FLAGS "--std=gnu++17 ${CMAKE_CXX_FLAGS}")

# Make sure gcc is new enough for full C++17 support.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0.0")
        message(FATAL_ERROR "GCC 7 or higher required for full C++17 support.")
    endif()
endif()

include_directories("${CMAKE_SOURCE_DIR}/include/") 

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(docs)

enable_testing()
add_test(SERVER tests/server)

