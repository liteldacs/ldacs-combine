//
// Created by 邹嘉旭 on 2025/4/24.
//

#include "ldacs_def.h"
#include "layer_p2p.h"

#include "gs_conn.h"
#include "ldcauc.h"

peer_service_t peer_service = {};

static field_desc handover_peer_ini_fields[] = {
    {ft_set, 1, "Forward", NULL},
    {ft_set, SAC_LEN, "AS_SAC", NULL},
    {ft_set, UA_LEN, "AS_UA", NULL},
    {ft_set, SAC_LEN, "GSS_SAC", NULL},
    {ft_set, SAC_LEN, "GST_SAC", NULL},
    {ft_set, 9, "NEXT_CO", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_end, 0, NULL, NULL},
};
struct_desc_t handover_peer_ini_desc = {"Handover Peer ini", handover_peer_ini_fields};

struct hashmap *init_peer_enode_map();

l_err set_peer_enode(peer_propt_node_t *en);

peer_propt_node_t *get_peer_enode_by_peerptr(peer_propt_t *ptr);;

l_err delete_peer_enode(uint16_t gs_sac, int8_t (*clear_func)(peer_propt_node_t *en));

l_err delete_peer_enode_by_peerptr(peer_propt_t *ptr, int8_t (*clear_func)(peer_propt_node_t *en));

void *p2p_conn_connect(net_ctx_t *ctx, char *remote_addr, int remote_port, int local_port) {
    peer_propt_t *peer_propt = malloc(sizeof(peer_propt_t));

    peer_propt->bc.remote_addr = strdup(remote_addr);
    peer_propt->bc.remote_port = remote_port;
    peer_propt->bc.local_port = local_port;

    if (init_basic_conn_client(&peer_propt->bc, ctx, LD_TCP_CLIENT) == FALSE) {
        return NULL;
    }
    return peer_propt;
}

l_err p2p_conn_accept(net_ctx_t *ctx, int fd, struct sockaddr_storage *saddr) {
    peer_propt_t *peer_propt = malloc(sizeof(peer_propt_t));

    if (init_basic_conn_server(&peer_propt->bc, ctx, LD_TCP_SERVER, fd, saddr) == FALSE) {
        log_error("Cannot initialize connection!");
        free(peer_propt);
        return LD_ERR_INTERNAL;
    }

    return LD_OK;
}

void p2p_conn_close(basic_conn_t *bc) {
    peer_propt_t *peer_propt = (peer_propt_t *) bc;
    if (!peer_propt) return;
    delete_peer_enode_by_peerptr(peer_propt, NULL);
    free(peer_propt);
    log_warn("Closing connection!");
}

l_err p2p_conn_recv(basic_conn_t *bc) {
    peer_propt_t *peer_propt = (peer_propt_t *) bc;
    log_buf(LOG_INFO, "RECV PEER", peer_propt->bc.read_pkt->ptr, peer_propt->bc.read_pkt->len);

    ho_peer_ini_t *ini = calloc(1, sizeof(ho_peer_ini_t));
    pb_stream pbs;
    zero(&pbs);
    init_pbs(&pbs, peer_propt->bc.read_pkt->ptr, peer_propt->bc.read_pkt->len, "GSNF IN");
    if (!in_struct(ini, &handover_peer_ini_desc, &pbs, NULL)) {
        log_error("Cannot parse gsnf pdu");
        free(ini);
        return LD_ERR_INTERNAL;
    }
    if (ini->is_ACK == FALSE) {
        gst_handover_request_handle(ini->AS_SAC, ini->AS_UA, ini->GSS_SAC, ini->GST_SAC);
    } else {
        // TODO:  send HO by CCCH
        log_info("Handover finished!");
        if (peer_service.ho_com_cb) peer_service.ho_com_cb(ini->AS_SAC, ini->GST_SAC, ini->NEXT_CO);
    }
    free(ini);
    return LD_OK;
}

l_err init_p2p_service(uint16_t server_port, peer_gs_t **peers, size_t peer_count, send_ho_com_cb send_ho_com) {
    peer_service.p2p_ctx = (net_ctx_t){
        .conn_handler = p2p_conn_connect,
        .accept_handler = p2p_conn_accept,
        .recv_handler = p2p_conn_recv,
        .send_handler = defalut_send_pkt,
        .close_handler = p2p_conn_close,
        .epoll_fd = core_epoll_create(0, -1),
    };

    peer_service.peer_map = init_peer_enode_map();
    peer_service.ho_com_cb = send_ho_com;
    server_entity_setup(server_port, &peer_service.p2p_ctx, LD_TCP_SERVER);
    for (int i = 0; i < peer_count; i++) {
        peer_gs_t *peer = peers[i];
        log_info("Peer[%d] `%s:%d` SAC: %d, listening on port: %d", i, peer->peer_addr, peer->peer_port, peer->peer_SAC,
                 server_port);
        peer_propt_t *propt = client_entity_setup(&peer_service.p2p_ctx, peer->peer_addr, peer->peer_port, 0);
        if (!propt) { return LD_ERR_NULL; }
        peer_propt_node_t *node = calloc(1, sizeof(peer_propt_node_t));
        node->propt = propt;
        node->GS_SAC = peer->peer_SAC;
        set_peer_enode(node);
    }

    pthread_create(&peer_service.p2p_thread, NULL, net_setup, &peer_service.p2p_ctx);
    pthread_detach(peer_service.p2p_thread);

    return LD_OK;
}

uint64_t hash_peer_enode(const void *item, uint64_t seed0, uint64_t seed1) {
    const peer_propt_node_t *node = item;
    return hashmap_sip(&node->GS_SAC, sizeof(uint16_t), seed0, seed1);
}

struct hashmap *init_peer_enode_map() {
    return hashmap_new(sizeof(peer_propt_node_t), 0, 0, 0,
                       hash_peer_enode, NULL, NULL, NULL);
}

l_err set_peer_enode(peer_propt_node_t *en) {
    if (!en) return LD_ERR_NULL;

    const void *ret = hashmap_set(peer_service.peer_map, en);
    /* !!!Do not free the previous entity !!! */
    free(en);
    return LD_OK;
}


peer_propt_node_t *get_peer_enode(uint16_t gs_sac) {
    return hashmap_get(peer_service.peer_map, &(peer_propt_node_t){
                           .GS_SAC = gs_sac,
                       });
}

peer_propt_t *get_peer_propt(uint16_t gs_sac) {
    peer_propt_node_t *peer_node;
    if ((peer_node = get_peer_enode(gs_sac)) == NULL) {
        return NULL;
    }
    return peer_node->propt;
}

peer_propt_node_t *get_peer_enode_by_peerptr(peer_propt_t *ptr) {
    size_t iter = 0;
    void *item;
    while (hashmap_iter(peer_service.peer_map, &iter, &item)) {
        peer_propt_node_t *node = item;
        if (node->propt == ptr)
            return node;
    }
    return NULL;
}

bool has_peer_enode(const uint16_t gs_sac) {
    return hashmap_get(peer_service.peer_map, &(peer_propt_node_t){
                           .GS_SAC = gs_sac,
                       }) != NULL;
}

l_err delete_peer_enode(uint16_t gs_sac, int8_t (*clear_func)(peer_propt_node_t *en)) {
    peer_propt_node_t *en = get_peer_enode(gs_sac);
    if (en) {
        if (clear_func) {
            clear_func(en);
        }
        hashmap_delete(peer_service.peer_map, en);
        return LD_OK;
    }
    return LD_ERR_INTERNAL;
}

l_err delete_peer_enode_by_peerptr(peer_propt_t *ptr, int8_t (*clear_func)(peer_propt_node_t *en)) {
    peer_propt_node_t *en = get_peer_enode_by_peerptr(ptr);
    if (en) {
        if (clear_func) {
            clear_func(en);
        }
        hashmap_delete(peer_service.peer_map, en);
        return LD_OK;
    }
    return LD_ERR_INTERNAL;
}
