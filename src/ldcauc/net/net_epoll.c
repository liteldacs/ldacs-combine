//
// Created by jiaxv on 23-7-9.
//

#include "ldcauc/net/net_epoll.h"

int net_epoll_add(int e_fd, void *conn_opt, uint32_t events,
                  struct epoll_event *pev) {
    FILL_EPOLL_EVENT(pev, conn_opt, events);
    return core_epoll_add(e_fd, (*(basic_conn_t **) conn_opt)->fd, pev);
}

void net_epoll_out(int e_fd, basic_conn_t *bc) {
    epoll_disable_in(e_fd, &bc->event, bc->fd);
    epoll_enable_out(e_fd, &bc->event, bc->fd);
    log_warn("OUT FD!! %d %d", e_fd, bc->fd);
}

void net_epoll_in(int e_fd, basic_conn_t *bc) {
    epoll_disable_out(e_fd, &bc->event, bc->fd);
    epoll_enable_in(e_fd, &bc->event, bc->fd);
}

// static struct timeval last_call_time;
// static const int TIME_INTERVAL_MS = 10; // 设置为10ms的间隔
