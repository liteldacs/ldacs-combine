//
// Created by 邹嘉旭 on 2025/4/24.
//

#ifndef P2P_H
#define P2P_H

#include "ldacs_def.h"
#include "connection.h"
#include "ld_config.h"



typedef struct peer_propt_s {
    basic_conn_t bc;
} peer_propt_t;

typedef struct peer_service_s {
    // p2p_propt_t *prop;
} peer_service_t;

l_err init_p2p_service(peer_gs_t **peers, size_t peer_count);

#endif //P2P_H
