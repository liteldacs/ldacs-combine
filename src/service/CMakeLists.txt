cmake_minimum_required(VERSION 3.20)
project(ldacs_service C)

set(CMAKE_C_STANDARD 11)

add_subdirectory(http)
add_subdirectory(terminal)
