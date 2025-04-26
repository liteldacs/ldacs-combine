//
// Created by 邹嘉旭 on 2025/4/24.
//

#include "ldacs_def.h"
#include "net/p2p.h"

net_ctx_t p2p_ctx;

void *p2p_client_conn(net_ctx_t *ctx, char *remote_addr, int remote_port, int local_port) {
    return NULL;
}

l_err init_p2p_service(uint16_t server_port, peer_gs_t **peers, size_t peer_count) {
    p2p_ctx = (net_ctx_t){
        .conn_handler = p2p_client_conn,
        .recv_handler = NULL,
        .close_handler = NULL,
        .epoll_fd = core_epoll_create(0, -1),
    };

    for (int i = 0; i < peer_count; i++) {
        peer_gs_t *peer = peers[i];
        log_warn("Peer[%d] %s:%d UA: %d, listening on port: %d", i, peer->peer_addr, peer->peer_port, peer->peer_UA,
                 server_port);
        client_entity_setup(&p2p_ctx, peer->peer_addr, peer->peer_port, 0);
    }

    return LD_OK;
}
