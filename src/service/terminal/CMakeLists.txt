cmake_minimum_required(VERSION 3.18)
project(ldacstermial)

set(CMAKE_C_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR}/include)

AUX_SOURCE_DIRECTORY(. SRC_LIST) #把当前目录(.)下所有源代码文件和头文件加入变量SRC_LIST
add_library(${PROJECT_NAME} STATIC ${SRC_LIST})
add_library(cli::ldacs_terminal ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include/net_core)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include/device)

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES output_name "ldacsterminal")
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
#INSTALL(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib/ldacs LIBRARY DESTINATION lib/ldacs RUNTIME DESTINATION bin)
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr/local/ldacs-cauc" CACHE PATH "Default install path" FORCE)
endif ()
INSTALL(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib/liteldacs LIBRARY DESTINATION lib/liteldacs RUNTIME DESTINATION bin)
