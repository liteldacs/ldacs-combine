//
// Created by jiaxv on 23-7-9.
//

#include <netdb.h>
#include <net/gs_conn.h>

#include "net/net_core.h"
#include <netinet/tcp.h>


static int connection_register(basic_conn_t *bc, int64_t factor);

static void connection_set_nodelay(basic_conn_t *bc);

#define ADDR_LEN (64/BITS_PER_BYTE)

/**
 * Store the basic_conn_t addresses into the propts in large end mode
 * @param start the start address of propts struct
 * @param addr address of basic_conn_t
 */
static void set_basic_conn_addr(uint8_t *start, void *addr) {
    uint64_t addr_int = (uint64_t) addr;
    for (size_t i = 0; i < ADDR_LEN; i++) {
        start[i] = (uint8_t) (addr_int >> (BITS_PER_BYTE * i));
    }
}

bool init_basic_conn(basic_conn_t *bc, net_ctx_t *ctx, sock_roles socket_role) {
    do {
        bc->fd = 0;
        bc->opt = ctx;
        bc->rp = get_role_propt(socket_role);
        bc->fd = bc->rp->handler(bc);

        if (bc->fd == ERROR) break;

        ABORT_ON(bc->opt->epoll_fd == 0 || bc->opt->epoll_fd == ERROR, "illegal epoll fd");

        if (connection_register(bc, time(NULL)) == ERROR) break;
        net_epoll_add(bc->opt->epoll_fd, bc, EPOLLIN | EPOLLET, &bc->event);
        set_fd_nonblocking(bc->fd);
        connection_set_nodelay(bc);

        zero(&bc->read_pkt);
        bc->write_pkts = lfqueue_init();

        return TRUE;
    } while (0);

    connection_close(bc);
    return FALSE;
}

static void connection_set_nodelay(basic_conn_t *bc) {
    static int enable = 1;
    setsockopt(bc->fd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
}

bool connecion_is_expired(basic_conn_t *bc, int timeout) {
    heap_t *conn_hp = get_heap(&bc->opt->hd_conns, bc);
    int64_t active_time = conn_hp->factor;
    return timeout ? (time(NULL) - active_time > timeout) : FALSE;
}

void connecion_set_reactivated(basic_conn_t *bc) {
    heap_t *conn_hp = get_heap(&bc->opt->hd_conns, bc);
    if (!conn_hp) return;
    conn_hp->factor = time(NULL); /* active_time */
    if (bc->rp->s_r & 1) heap_bubble_down(&bc->opt->hd_conns, conn_hp->heap_idx);
}

void connecion_set_expired(basic_conn_t *bc) {
    heap_t *conn_hp = get_heap(&bc->opt->hd_conns, bc);
    if (!conn_hp) return;
    conn_hp->factor = 0; // very old time
    if (bc->rp->s_r & 1) heap_bubble_up(&bc->opt->hd_conns, conn_hp->heap_idx);
    connection_close(bc);
}

static int connection_register(basic_conn_t *bc, int64_t factor) {
    if (bc->opt->hd_conns.heap_size >= MAX_HEAP) {
        return ERROR;
    }
    return heap_insert(&bc->opt->hd_conns, bc, factor);
}

void connection_unregister(basic_conn_t *bc) {
    assert(bc->opt->hd_conns.heap_size >= 1);

    heap_t *conn_hp = get_heap(&bc->opt->hd_conns, bc);
    int heap_idx = conn_hp->heap_idx;
    bc->opt->hd_conns.hps[heap_idx] = bc->opt->hd_conns.hps[bc->opt->hd_conns.heap_size - 1];
    bc->opt->hd_conns.hps[heap_idx]->heap_idx = heap_idx;
    bc->opt->hd_conns.heap_size--;

    fprintf(stderr, "HEAP SIZE: %d\n", bc->opt->hd_conns.heap_size);
    heap_bubble_down(&bc->opt->hd_conns, heap_idx);
    if (bc->opt->close_handler) bc->opt->close_handler(bc);
}


/* close connection, free memory */
void connection_close(basic_conn_t *bc) {
    passert(bc != NULL);
    ABORT_ON(bc->fd == ERROR, "FD ERROR");


    core_epoll_del(bc->opt->epoll_fd, bc->fd, 0, NULL);
    if (close(bc->fd) == ERROR) {
        log_info("The remote has closed, EXIT!");
        //raise(SIGINT); /* terminal, send signal */
    }

    connection_unregister(bc);
}

void server_connection_prune(net_ctx_t *opt) {
    while (opt->hd_conns.heap_size > 0 && opt->timeout) {
        basic_conn_t *bc = opt->hd_conns.hps[0]->obj;
        int64_t active_time = opt->hd_conns.hps[0]->factor;
        if (time(NULL) - active_time >= opt->timeout) {
            log_info("prune %p %d\n", bc, opt->hd_conns.heap_size);
            connection_close(bc);
        } else
            break;
    }
}
