//
// Created by 邹嘉旭 on 2024/11/9.
//

#ifndef GS_CONN_H
#define GS_CONN_H
#include "net/connection.h"

typedef struct gs_tcp_propt_s {
    basic_conn_t bc;
} gs_tcp_propt_t;

struct shared_key_temp {
    int32_t uas;
    uint8_t key[4];
};

extern const struct shared_key_temp shared_keys[];

bool recv_gs_pkt(basic_conn_t *bc);

bool send_gs_pkt(basic_conn_t *bc);

bool forward_gs_pkt(basic_conn_t *bc);

bool reset_gs_conn(basic_conn_t *bc);

void close_gs_conn(basic_conn_t *bc);

l_err gs_conn_accept(net_ctx_t *ctx);

void *gs_conn_connect(net_ctx_t *ctx, char *remote_addr, int remote_port, int local_port);

#endif //GS_CONN_H
