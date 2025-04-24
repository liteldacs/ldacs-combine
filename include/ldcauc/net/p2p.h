//
// Created by 邹嘉旭 on 2025/4/24.
//

#ifndef P2P_H
#define P2P_H

#include "ldacs_def.h"
#include "connection.h"

typedef struct p2p_propt_s {
    basic_conn_t bc;
} p2p_propt_t;

typedef struct p2p_service_s {
    p2p_propt_t *prop;

}p2p_service_t;

#endif //P2P_H
