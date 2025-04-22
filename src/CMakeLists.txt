cmake_minimum_required(VERSION 3.20)
project(ldacs_sim_src C)

set(CMAKE_C_STANDARD 11)


add_subdirectory(roles)
add_subdirectory(layer)
add_subdirectory(device)
add_subdirectory(service)
add_subdirectory(ldcauc)


