cmake_minimum_required(VERSION 3.20)
project(ldacstool)

set(CMAKE_C_STANDARD 11)

AUX_SOURCE_DIRECTORY(. SRC_LIST) #把当前目录(.)下所有源代码文件和头文件加入变量SRC_LIST
add_library(${PROJECT_NAME} STATIC ${SRC_LIST})
add_library(cli::ldacs_tools ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include/roles)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include/net_core)

SET_TARGET_PROPERTIES(ldacstool PROPERTIES output_name "ldacstool")
SET_TARGET_PROPERTIES(ldacstool PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr/local/ldacs-cauc" CACHE PATH "Default install path" FORCE)
endif ()
INSTALL(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib/liteldacs LIBRARY DESTINATION lib/liteldacs RUNTIME DESTINATION bin)
