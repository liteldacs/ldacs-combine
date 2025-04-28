//
// Created by 邹嘉旭 on 2025/4/24.
//

#ifndef P2P_H
#define P2P_H

#include "ldacs_def.h"
#include "connection.h"
#include "ld_config.h"

#include <ld_hashmap.h>


typedef struct peer_propt_s {
    basic_conn_t bc;
    uint16_t SAC;
} peer_propt_t;

typedef struct peer_propt_node_s {
    peer_propt_t *propt;
    uint16_t GS_SAC;
} peer_propt_node_t;

typedef struct peer_service_s {
    struct hashmap *peer_map;
    net_ctx_t p2p_ctx;
    pthread_t p2p_thread;
} peer_service_t;

l_err init_p2p_service(uint16_t server_port, peer_gs_t **peers, size_t peer_count);

#endif //P2P_H
