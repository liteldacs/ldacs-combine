cmake_minimum_required(VERSION 3.18)
project(ldacs_sim_src C)

set(CMAKE_C_STANDARD 11)


add_subdirectory(roles)
add_subdirectory(layer)
add_subdirectory(device)
add_subdirectory(ldacs_security)
add_subdirectory(service)
add_subdirectory(net_core)
add_subdirectory(ldcauc)


