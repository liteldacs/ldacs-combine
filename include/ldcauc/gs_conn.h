//
// Created by 邹嘉旭 on 2024/11/9.
//

#ifndef GS_CONN_H
#define GS_CONN_H
#include "net/connection.h"

typedef struct gs_propt_s {
    basic_conn_t bc;
    uint16_t GS_SAC;
} gs_propt_t;

typedef struct gs_conn_define_s {
    char *addr;
    int port;
    uint16_t GS_SAC;
} gs_conn_define_t;

typedef struct gs_conn_service_s {
    gs_conn_define_t conn_defines[10];
} gs_conn_service_t;


bool recv_gs_pkt(basic_conn_t *bc);

bool send_gs_pkt(basic_conn_t *bc);

bool forward_gs_pkt(basic_conn_t *bc);

bool reset_gs_conn(basic_conn_t *bc);

void close_gs_conn(basic_conn_t *bc);

l_err gs_conn_accept(net_ctx_t *ctx);

void *gs_conn_connect(net_ctx_t *ctx, char *remote_addr, int remote_port, int local_port);

#endif //GS_CONN_H
