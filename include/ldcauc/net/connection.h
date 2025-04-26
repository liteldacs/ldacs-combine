//
// Created by jiaxv on 23-7-9.
//

#ifndef TEST_CLIENT_CONNECTION_H
#define TEST_CLIENT_CONNECTION_H

#include <ld_log.h>
#include <ld_heap.h>
#include <ld_epoll.h>
#include <passert.h>
#include <ld_mqueue.h>
#include "net/net_core.h"


typedef struct basic_conn_s {
    int fd; /* connection_s fd */
    struct epoll_event event; /* epoll event */
    struct sockaddr_storage saddr; /* IP socket address */
    buffer_t read_pkt; /* Receive packet */
    lfqueue_t *write_pkts;
    bool trans_done;
    const struct role_propt *rp;
    struct net_ctx_s *opt;

    //client
    char *remote_addr;
    int remote_port;
    int local_port;
} basic_conn_t;


bool init_basic_conn(basic_conn_t *bc, net_ctx_t *ctx, sock_roles socket_role);

bool connecion_is_expired(basic_conn_t *bcp, int timeout);

void connection_close(basic_conn_t *bc);

void connecion_set_reactivated(basic_conn_t *bdp);

void connecion_set_expired(basic_conn_t *bcp);

void server_connection_prune(net_ctx_t *opt);


#endif //TEST_CLIENT_CONNECTION_H
