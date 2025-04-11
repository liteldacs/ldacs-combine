//
// Created by 邹嘉旭 on 2025/3/30.
//

#ifndef LDCAUC_H
#define LDCAUC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define LDCAUC_OK (0)
#define LDCAUC_FAIL (-1)
#define LDCAUC_WRONG_PARA (-2)
#define LDCAUC_NULL (-3)
#define LDCAUC_INTERNAL_ERROR (-4)

#define ROLE_AS 1
#define ROLE_GS 2
#define ROLE_SGW 4

#define PROTECT_VERSION 1

enum SEC_ALG_MACLEN {
    SEC_MACLEN_INVAILD = 0x0,
    SEC_MACLEN_96 = 0x1,
    SEC_MACLEN_128 = 0x2,
    SEC_MACLEN_64 = 0x3,
    SEC_MACLEN_256 = 0x4,
};

#define get_sec_maclen(en)({    \
    int ret;                    \
    switch(en){                 \
        case 0x1:               \
            ret = 12;          \
            break;              \
        case 0x2:               \
            ret = 16;          \
            break;              \
        case 0x3:               \
            ret = 8;          \
            break;              \
        case 0x4:               \
            ret = 32;          \
            break;              \
        default:                \
            ret = 0;            \
            break;              \
    };                          \
    ret;        \
})

typedef int8_t (*finish_auth)();

typedef int8_t (*trans_snp)(uint16_t AS_SAC, uint16_t GS_SAC, uint8_t *buf, size_t buf_len);

/**
 * \brief  AS初始化SNF层
 * @param[in] finish_auth   认证完成回调函数
 * @param[in] trans_snp     LME->SNP 回调函数
 */
int8_t init_as_snf_layer(finish_auth finish_auth, trans_snp trans_snp);

/**
 * \brief  GS初始化SNF层
 * @param[in] GS_SAC        GS SAC
 * @param[in] gsnf_addr     GSC/网关IPv6地址
 * @param[in] trans_snp     LME->SNP 回调函数
 */
int8_t init_gs_snf_layer(uint16_t GS_SAC, const char *gsnf_addr, uint16_t gsnf_port,
                         trans_snp trans_snp);

int8_t init_gs_snf_layer_unmerged(uint16_t GS_SAC, const char *gsnf_addr, uint16_t gsnf_port,
                                  trans_snp trans_snp);

/**
 * \brief  网关初始化SNF层
 * @param[in] listen_port     监听端口
 */
int8_t init_sgw_snf_layer(uint16_t listen_port);

/**
 * \brief  清理SNF层数据并释放对应内存
 */
int8_t destory_snf_layer();

/**
 * \brief AUTH状态转换流程
 * AS进入LME-AUTH状态时调用函数，该函数初始化AS唯一的SNF实体，并启动内部AUTHC流程
 * @param[in] role      角色（ROLE_AS、ROLE_GS、ROLE_SGW）
 * @param[in] AS_SAC    AS SAC
 * @param[in] AS_UA     AS UA
 * @param[in] GS_SAC    GS SAC
 */
int8_t snf_LME_AUTH(uint8_t role, uint16_t AS_SAC, uint32_t AS_UA, uint16_t GS_SAC);

/**
 * \brief AUTH状态转换流程
 * AS进入LME-AUTH状态时调用函数，该函数初始化AS唯一的SNF实体，并启动内部AUTHC流程
 * @param[in] role      角色（ROLE_AS、ROLE_GS、ROLE_SGW）
 * @param[in] AS_SAC    AS SAC
 * @param[in] AS_UA     AS UA
 * @param[in] GS_SAC    GS SAC
 */
int8_t upload_snf(bool is_valid, uint16_t AS_SAC, uint8_t *buf, size_t buf_len);

int8_t register_snf_en(uint8_t role, uint16_t AS_SAC, uint32_t AS_UA, uint16_t GS_SAC);

int8_t unregister_snf_en(uint16_t SAC);

uint64_t generate_urand(size_t rand_bits_sz);

#endif //LDCAUC_H
