cmake_minimum_required(VERSION 3.20)
project(ldacsservice C)

set(CMAKE_C_STANDARD 11)

add_subdirectory(http)
add_subdirectory(terminal)
add_subdirectory(dashboard)

include_directories(${PROJECT_SOURCE_DIR}/include)

AUX_SOURCE_DIRECTORY(. SRC_LIST)
add_library(${PROJECT_NAME} STATIC ${SRC_LIST}
        service.c
)
add_library(cli::ldacs_service ALIAS ${PROJECT_NAME})

SET_TARGET_PROPERTIES(ldacsservice PROPERTIES output_name "ldacsservice")
SET_TARGET_PROPERTIES(ldacsservice PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
#INSTALL(TARGETS ldacslayer ARCHIVE DESTINATION lib/ldacs LIBRARY DESTINATION lib/ldacs RUNTIME DESTINATION bin)
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr/local/ldacs-cauc" CACHE PATH "Default install path" FORCE)
endif ()
INSTALL(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib/liteldacs LIBRARY DESTINATION lib/liteldacs RUNTIME DESTINATION bin)

