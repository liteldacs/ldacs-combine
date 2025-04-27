//
// Created by 邹嘉旭 on 2024/11/9.
//


#include "net/gs_conn.h"
#include <ld_config.h>
#include "net/net.h"

bool send_gs_pkt(basic_conn_t *bcp) {
    return TRUE;
}

void *gs_conn_connect(net_ctx_t *ctx, char *remote_addr, int remote_port, int local_port) {
    gs_tcp_propt_t *gs_conn = malloc(sizeof(gs_tcp_propt_t));

    gs_conn->bc.remote_addr = strdup(remote_addr);
    gs_conn->bc.remote_port = remote_port;
    gs_conn->bc.local_port = local_port;

    if (init_basic_conn(&gs_conn->bc, ctx, LD_TCP_CLIENT) == FALSE) {
        return NULL;
    }

    return gs_conn;
}

l_err gs_conn_accept(net_ctx_t *ctx) {
    gs_tcp_propt_t *gs_conn = malloc(sizeof(gs_tcp_propt_t));

    if (init_basic_conn(&gs_conn->bc, ctx, LD_TCP_SERVER) == FALSE) {
        log_error("Cannot initialize connection!");
        free(gs_conn);
        return LD_ERR_INTERNAL;
    }

    log_warn("!!!! %d %d", ntohs(((struct sockaddr_in *)&gs_conn->bc.saddr)->sin_port));

    return LD_OK;
}

bool reset_gs_conn(basic_conn_t *bc) {
    gs_tcp_propt_t *mlt_ld = (gs_tcp_propt_t *) bc;
    return TRUE;
}

void close_gs_conn(basic_conn_t *bc) {
    gs_tcp_propt_t *gs_conn = (gs_tcp_propt_t *) bc;
    free(gs_conn);
    log_warn("Closing connection!");
}

