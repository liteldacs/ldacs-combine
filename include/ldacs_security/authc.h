//
// Created by 邹嘉旭 on 2024/3/21.
//

#ifndef LDACS_SIM_AUTH_H
#define LDACS_SIM_AUTH_H

#include "secure_core.h"


#pragma pack(1)
#pragma pack()

#define get_klen(en)({  \
    int ret;            \
    switch (en){        \
        case AUTHC_KLEN_128:               \
            ret = 16;          \
            break;              \
        case AUTHC_KLEN_256:               \
            ret = 32;          \
            break;              \
        default:                \
            ret = 0;            \
            break;              \
    };                       \
    ret;                       \
})


#endif //LDACS_SIM_AUTH_H
