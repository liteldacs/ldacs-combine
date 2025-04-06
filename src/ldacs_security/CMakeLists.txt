cmake_minimum_required(VERSION 3.18)
project(ldacssecure)

set(CMAKE_C_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR}/include)

AUX_SOURCE_DIRECTORY(. SRC_LIST)
add_library(${PROJECT_NAME} STATIC ${SRC_LIST})
add_library(cli::ldacs_security ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include/ldacs_security)

cmake_host_system_information(RESULT PRETTY_NAME QUERY DISTRIB_PRETTY_NAME)
cmake_host_system_information(RESULT HOST_NAME QUERY HOSTNAME)
IF (${HOST_NAME} MATCHES "gateway" OR ${HOST_NAME} MATCHES "gsc")
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC km_src sdf)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_CRYCARD)
ELSEIF (${HOST_NAME} MATCHES "gs" OR ${HOST_NAME} MATCHES "as")
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC km_src piico_ccmu)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_CRYCARD)
ELSE ()
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC km_src gmssl)
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNUSE_CRYCARD)
ENDIF ()

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES output_name "ldacssecurity")
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
#INSTALL(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib/ldacs LIBRARY DESTINATION lib/ldacs RUNTIME DESTINATION bin)
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr/local/ldacs-cauc" CACHE PATH "Default install path" FORCE)
endif ()
INSTALL(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib/liteldacs LIBRARY DESTINATION lib/liteldacs RUNTIME DESTINATION bin)
