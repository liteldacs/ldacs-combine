//
// Created by 邹嘉旭 on 2025/4/24.
//

#include "ldacs_def.h"
#include "net/p2p.h"

peer_service_t peer_service = {};


struct hashmap *init_peer_enode_map();
const void *set_peer_enode(peer_propt_t *en);

void *p2p_conn_connect(net_ctx_t *ctx, char *remote_addr, int remote_port, int local_port) {
    peer_propt_t *peer_propt = malloc(sizeof(peer_propt_t));

    peer_propt->bc.remote_addr = strdup(remote_addr);
    peer_propt->bc.remote_port = remote_port;
    peer_propt->bc.local_port = local_port;

    if (init_basic_conn(&peer_propt->bc, ctx, LD_TCP_CLIENT) == FALSE) {
        return NULL;
    }
    return peer_propt;
}

l_err p2p_conn_accept(net_ctx_t *ctx) {
    peer_propt_t *peer_propt = malloc(sizeof(peer_propt_t));

    if (init_basic_conn(&peer_propt->bc, ctx, LD_TCP_SERVER) == FALSE) {
        log_error("Cannot initialize connection!");
        free(peer_propt);
        return LD_ERR_INTERNAL;
    }

    log_warn("!!!! %d %d", ntohs(((struct sockaddr_in *)&peer_propt->bc.saddr)->sin_port));

    return LD_OK;
}

l_err init_p2p_service(uint16_t server_port, peer_gs_t **peers, size_t peer_count) {
    peer_service.p2p_ctx = (net_ctx_t){
        .conn_handler = p2p_conn_connect,
        .accept_handler = p2p_conn_accept,
        .recv_handler = NULL,
        .close_handler = NULL,
        .epoll_fd = core_epoll_create(0, -1),
    };

    peer_service.peer_map = init_peer_enode_map();

    for (int i = 0; i < peer_count; i++) {
        peer_gs_t *peer = peers[i];
        log_warn("Peer[%d] %s:%d UA: %d, listening on port: %d", i, peer->peer_addr, peer->peer_port, peer->peer_UA,
                 server_port);
        peer_propt_t *propt = client_entity_setup(&peer_service.p2p_ctx, peer->peer_addr, peer->peer_port, 0);
        propt->SAC = peer->peer_UA;

        set_peer_enode(propt);
    }

    server_entity_setup(server_port, &peer_service.p2p_ctx);

    return LD_OK;
}

uint64_t hash_peer_enode(const void *item, uint64_t seed0, uint64_t seed1) {
    const peer_propt_t *node = item;
    return hashmap_sip(&node->SAC, sizeof(uint16_t), seed0, seed1);
}

struct hashmap *init_peer_enode_map() {
    return hashmap_new(sizeof(peer_propt_t), 0, 0, 0,
                       hash_peer_enode, NULL, NULL, NULL);
}

const void *set_peer_enode(peer_propt_t *en) {
    if (!en) return NULL;

    const void *ret = hashmap_set(peer_service.peer_map, en);
    free(en);
    return ret;
}

peer_propt_t *get_peer_enode(const uint16_t gs_sac) {
    return hashmap_get(peer_service.peer_map, &(peer_propt_t){
                           .SAC = gs_sac,
                       });
}

bool has_peer_enode(const uint16_t gs_sac) {
    return hashmap_get(peer_service.peer_map, &(peer_propt_t){
                           .SAC = gs_sac,
                       }) != NULL;
}

l_err delete_peer_enode(uint16_t gs_sac, int8_t (*clear_func)(peer_propt_t *en)) {
    peer_propt_t *en = get_peer_enode(gs_sac);
    if (en) {
        if (clear_func) {
            clear_func(en);
        }
        hashmap_delete(peer_service.peer_map, en);
        return LD_OK;
    }
    return LD_ERR_INTERNAL;
}


