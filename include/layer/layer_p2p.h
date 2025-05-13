//
// Created by 邹嘉旭 on 2025/4/24.
//

#ifndef P2P_H
#define P2P_H

#include <ldacs_def.h>
#include <ld_config.h>
#include <ld_net.h>
#include <ld_hashmap.h>

typedef l_err (*send_ho_com_cb)();

typedef struct peer_propt_s {
    basic_conn_t bc;
} peer_propt_t;

typedef struct peer_propt_node_s {
    peer_propt_t *propt;
    uint16_t GS_SAC;
} peer_propt_node_t;

typedef struct peer_service_s {
    struct hashmap *peer_map;
    net_ctx_t p2p_ctx;
    pthread_t p2p_thread;
    send_ho_com_cb ho_com_cb;
} peer_service_t;

#pragma pack(1)
typedef struct ho_peer_ini_s {
    uint8_t is_ACK;
    uint16_t AS_SAC;
    uint32_t AS_UA;
    uint16_t GSS_SAC;
    uint16_t GST_SAC;
} ho_peer_ini_t;
#pragma pack()

extern struct_desc_t handover_peer_ini_desc;


l_err init_p2p_service(uint16_t server_port, peer_gs_t **peers, size_t peer_count, send_ho_com_cb send_ho_com);

peer_propt_node_t *get_peer_enode(const uint16_t gs_sac);

peer_propt_t *get_peer_propt(uint16_t gs_sac);

#endif //P2P_H
