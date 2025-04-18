cmake_minimum_required(VERSION 3.20)
project(ldacslayer)

set(CMAKE_C_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR}/include)

AUX_SOURCE_DIRECTORY(. SRC_LIST)
add_library(${PROJECT_NAME} STATIC ${SRC_LIST}
        phy/phy.c
        phy/phy_sim_json.c
        phy/phy_sim_real.c
        phy/phy_define.c
        mac/mac.c
        mac/mac_util.c
        mac/mac_define.c
        dls/dls.c
        dls/dls_entity.c
        dls/dls_util.c
        dls/dls_define.c
        lme/lme.c
        lme/lme_mms.c
        lme/lme_rms.c
        lme/lme_util.c
        lme/lme_define.c
        snp/ldacs_snp.c
        snp/snp_define.c
        layer_interface.c
        snp/snp_util.c
        rcu/rcu.c
        rcu/rcu_define.c
)
add_library(cli::ldacs_layer ALIAS ${PROJECT_NAME})


ADD_COMPILE_DEFINITIONS(${PROJECT_NAME} PRIVATE HAS_SGW HAS_GSC)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include/layer)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include/ldacs_security)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include/net_core)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include/tools)

SET_TARGET_PROPERTIES(ldacslayer PROPERTIES output_name "ldacslayer")
SET_TARGET_PROPERTIES(ldacslayer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
#INSTALL(TARGETS ldacslayer ARCHIVE DESTINATION lib/ldacs LIBRARY DESTINATION lib/ldacs RUNTIME DESTINATION bin)
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr/local/ldacs-cauc" CACHE PATH "Default install path" FORCE)
endif ()
INSTALL(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib/liteldacs LIBRARY DESTINATION lib/liteldacs RUNTIME DESTINATION bin)
