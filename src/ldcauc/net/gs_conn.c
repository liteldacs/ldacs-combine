//
// Created by 邹嘉旭 on 2024/11/9.
//


#include "net/gs_conn.h"

#include <ld_config.h>

#include "net/net.h"


void *init_gs_conn(net_opt_t *net_opt, sock_roles socket_role) {
    gs_tcp_propt_t *gs_conn = malloc(sizeof(gs_tcp_propt_t));

    if (init_basic_conn(&gs_conn->bc, net_opt, socket_role) == FALSE) {
        return NULL;
    }
    return gs_conn;
}

bool send_gs_pkt(basic_conn_t *bcp) {
    return TRUE;
}

l_err gs_conn_accept(net_opt_t *net_opt) {
    if (init_gs_conn(net_opt, LD_TCP_SERVER) == NULL) {
        log_error("Cannot initialize connection!");
        return LD_ERR_INTERNAL;
    }
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

