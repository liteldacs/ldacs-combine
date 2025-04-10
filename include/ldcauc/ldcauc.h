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


uint64_t generate_urand(size_t rand_bits_sz);

int8_t init_as_snf_layer(finish_auth finish_auth, trans_snp trans_snp);

int8_t init_gs_snf_layer(uint16_t GS_SAC, const char *gsnf_addr, uint16_t gsnf_port,
                         trans_snp trans_snp);

int8_t init_gs_snf_layer_unmerged(uint16_t GS_SAC, const char *gsnf_addr, uint16_t gsnf_port,
                                  trans_snp trans_snp);

int8_t init_sgw_snf_layer(uint16_t listen_port);

int8_t destory_snf_layer();

int8_t snf_LME_AUTH(uint8_t role, uint16_t AS_SAC, uint32_t AS_UA, uint16_t GS_SAC);

int8_t upload_snf(bool is_valid, uint16_t AS_SAC, uint8_t *buf, size_t buf_len);

int8_t register_snf_en(uint8_t role, uint16_t AS_SAC, uint32_t AS_UA, uint16_t GS_SAC);

int8_t unregister_snf_en(uint16_t SAC);

#endif //LDCAUC_H
