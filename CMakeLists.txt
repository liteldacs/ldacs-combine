cmake_minimum_required(VERSION 3.18)
project(ldacs-combine C)

set(CMAKE_C_STANDARD 11)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

cmake_host_system_information(RESULT PRETTY_NAME QUERY DISTRIB_PRETTY_NAME)
IF (${PRETTY_NAME} MATCHES "Ubuntu")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=leak -fno-omit-frame-pointer -g")
    #    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=leak -g")
ENDIF ()


INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/msg_core)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/ldacs_security)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/net_core)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/roles)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/layer)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/device)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/tools)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/service/http)
INCLUDE_DIRECTORIES(/usr/local/sqlite/include)
#INCLUDE_DIRECTORIES(/usr/local/include/ldacs)


add_executable(${PROJECT_NAME} main.c)
add_executable(cli::test_client ALIAS ${PROJECT_NAME})


target_link_libraries(${PROJECT_NAME} LINK_PUBLIC cli::ldacs_roles cli::ldacs_layer cli::ldacs_http cli::ldacs_terminal cli::ldacs_device
        event event_pthreads ldcauc)

find_package(SQLite3 REQUIRED)
if (SQLite3_FOUND)
    include_directories(${SQLite3_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${SQLite3_LIBRARIES})
    message(${SQLite3_VERSION})
endif (SQLite3_FOUND)

find_package(LiteLdacsSdk REQUIRED)
if (LiteLdacsSdk_FOUND)
    include_directories(${LiteLdacsSdk_INCLUDE_DIRS})
    link_libraries(liteldacssdk::liteldacssdk)
endif ()

find_package(Ldacscrypto REQUIRED)
if (Ldacscrypto_FOUND)
    include_directories(${Ldacscrypto_INCLUDE_DIRS})
    link_libraries(ldacscrypto::ldacscrypto)
endif ()

#find_package(Ldcauc REQUIRED)
#if (Ldcauc_FOUND)
#    include_directories(${Ldcauc_INCLUDE_DIRS})
#    link_libraries(ldcauc::ldcauc)
#endif ()

add_subdirectory(src)

# 生成 config.h 头文件
configure_file(
        ${CMAKE_SOURCE_DIR}/in/cconfig.h.in
        ${CMAKE_SOURCE_DIR}/include/cconfig.h
)

message(${CMAKE_INSTALL_PREFIX})
INSTALL(CODE "
    # 创建目录（包含父目录）
    file(MAKE_DIRECTORY \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/log\")
    # 设置目录权限：所有者读写执行，组和其他人读执行
    file(CHMOD \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}\"
        DIRECTORY_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    )
    file(CHMOD \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/log\"
        DIRECTORY_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE GROUP_WRITE
        WORLD_READ WORLD_EXECUTE WORLD_WRITE
    )

    file(COPY \"${CMAKE_SOURCE_DIR}/config\"
    DESTINATION \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}\"
    FILE_PERMISSIONS
        OWNER_READ OWNER_WRITE
        GROUP_READ
        WORLD_READ
    )
    file(CHMOD \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/config\"
        DIRECTORY_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    )
")


if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr/local/ldacs-cauc" CACHE PATH "Default install path" FORCE)
endif ()
INSTALL(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib/liteldacs LIBRARY DESTINATION lib/liteldacs RUNTIME DESTINATION /usr/local/bin)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include/liteldacs)

