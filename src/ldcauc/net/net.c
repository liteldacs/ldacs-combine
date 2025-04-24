//
// Created by jiaxv on 23-7-9.
//
#include <netinet/tcp.h>

#include "net/gs_conn.h"
#include "net/net_core.h"

#define BACKLOG 1024
#define RECONNECT 20


static int make_std_tcp_connect(struct sockaddr_in *to_conn_addr, char *addr, int remote_port, int local_port) {
    struct in_addr s;
    int fd;
    int enable = SO_REUSEADDR;

    struct timeval timeout = {
        .tv_sec = 5, /* after 5 seconds connect() will timeout  */
        .tv_usec = 0,
    };

    inet_pton(AF_INET, addr, &s);
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
        return ERROR;

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    zero(to_conn_addr);
    to_conn_addr->sin_family = AF_INET;
    to_conn_addr->sin_port = htons(remote_port);
    // to_conn_addr->sin_addr = s;
    memcpy(&to_conn_addr->sin_addr, &s, sizeof(s));

    /* 绑定本地端口 */
    struct sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(local_port); // 转换为网络字节序
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 允许任意本地地址绑定

    if (bind(fd, (struct sockaddr *) &local_addr, sizeof(local_addr)) == -1) {
        perror("bind failed");
        close(fd);
        return -1;
    }

    //TODO: 改成死循环，持续1min
    int i = RECONNECT;
    while (i--) {
        log_info("Trying to connect to GSC %s:%d  for %d time(s).", addr, remote_port, RECONNECT - i);
        if (connect(fd, (struct sockaddr *) to_conn_addr, sizeof(struct sockaddr_in)) >= 0) {
            log_info("Connected");
            return fd;
        }
        sleep(1);
    }

    log_error("Failed to connect. Exit...");

    return ERROR;
}

static int make_std_tcpv6_connect(struct sockaddr_in6 *to_conn_addr, char *addr, int remote_port, int local_port) {
    struct in6_addr s;
    int fd;
    int enable = SO_REUSEADDR;

    struct timeval timeout = {
        .tv_sec = 5, /* after 5 seconds connect() will timeout  */
        .tv_usec = 0,
    };

    inet_pton(AF_INET6, addr, &s);
    if ((fd = socket(AF_INET6, SOCK_STREAM, 0)) == ERROR)
        return ERROR;

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    zero(to_conn_addr);
    to_conn_addr->sin6_family = AF_INET6;
    to_conn_addr->sin6_port = htons(remote_port);
    // to_conn_addr->sin_addr = s;
    memcpy(&to_conn_addr->sin6_addr, &s, sizeof(s));


    /* 绑定本地端口 */
    struct sockaddr_in6 local_addr;
    local_addr.sin6_family = AF_INET6;
    local_addr.sin6_port = htons(local_port); // 转换为网络字节序
    local_addr.sin6_addr = in6addr_any; // 允许任意本地地址绑定

    if (bind(fd, (struct sockaddr *) &local_addr, sizeof(local_addr)) == -1) {
        perror("bind failed");
        close(fd);
        return -1;
    }
    //TODO: 改成死循环，持续1min
    int i = RECONNECT;
    while (i--) {
        log_info("Trying to connect to GSC  %s:%d  for %d time(s).", addr, remote_port, RECONNECT - i);
        if (connect(fd, (struct sockaddr *) to_conn_addr, sizeof(struct sockaddr_in6)) >= 0) {
            log_info("Connected");
            return fd;
        }
        sleep(1);
    }

    log_error("Failed to connect. Exit...");

    return ERROR;
}

static int make_std_tcp_server(uint16_t port) {
    struct sockaddr_in saddr;
    int n_fd;

    if ((n_fd = socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
        return ERROR;

    int enable = SO_REUSEADDR;
    setsockopt(n_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    // if (config.worker > 1) {
    //     // since linux 3.9
    //     setsockopt(n_fd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(enable));
    // }

    zero(&saddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(n_fd, (struct sockaddr *) &saddr, sizeof(saddr)) != OK)
        return ERROR;
    if (listen(n_fd, BACKLOG) != OK)
        return ERROR;

    return n_fd;
}

static int make_std_tcpv6_server(uint16_t port) {
    struct sockaddr_in6 saddr;
    int n_fd;

    if ((n_fd = socket(AF_INET6, SOCK_STREAM, 0)) == ERROR)
        return ERROR;

    int enable = 1; // SO_REUSEADDR and SO_REUSEPORT flag value

    setsockopt(n_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

    // if (config.worker > 1) {
    //     // since linux 3.9
    //     setsockopt(n_fd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(enable));
    // }

    // Allow both IPv6 and IPv4 connections on this socket.
    // If you want to restrict it to IPv6 only, set this option to 1.
    int v6only = 0;
    setsockopt(n_fd, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only));

    memset(&saddr, 0, sizeof(saddr)); // Zero out the structure
    saddr.sin6_family = AF_INET6;
    saddr.sin6_port = htons(port);
    memcpy(&saddr.sin6_addr, &in6addr_any, sizeof(in6addr_any));

    if (bind(n_fd, (struct sockaddr *) &saddr, sizeof(saddr)) != OK) {
        return ERROR;
    }
    if (listen(n_fd, BACKLOG) != OK)
        return ERROR;

    return n_fd;
}

static int make_std_tcp_accept(basic_conn_t *bc) {
    struct sockaddr_in *to_conn_addr = (struct sockaddr_in *) &bc->saddr;
    int fd;
    socklen_t saddrlen = sizeof(struct sockaddr_in);
    if (bc->opt->server_fd == DEFAULT_FD) return DEFAULT_FD;
    while ((fd = accept(bc->opt->server_fd, (struct sockaddr *) to_conn_addr, &saddrlen)) == ERROR) {
    }
    return fd;
}

static int make_std_tcpv6_accept(basic_conn_t *bc) {
    struct sockaddr_in6 *to_conn_addr = (struct sockaddr_in6 *) &bc->saddr;
    int fd;
    socklen_t saddrlen = sizeof(struct sockaddr_in6);
    if (bc->opt->server_fd == DEFAULT_FD) return DEFAULT_FD;
    while ((fd = accept(bc->opt->server_fd, (struct sockaddr *) to_conn_addr, &saddrlen)) == ERROR) {
        if (errno != EINTR) {
            // 如果不是由信号中断，则报告错误并退出
            perror("accept");
            return ERROR;
        }
    }

    return fd;
}

static int init_std_tcp_conn_handler(basic_conn_t *bc) {
    return make_std_tcp_connect((struct sockaddr_in *) &bc->saddr, bc->opt->addr, bc->opt->remote_port,
                                bc->opt->local_port);
}

static int init_std_tcpv6_conn_handler(basic_conn_t *bc) {
    return make_std_tcpv6_connect((struct sockaddr_in6 *) &bc->saddr, bc->opt->addr, bc->opt->remote_port,
                                  bc->opt->local_port);
}

static int init_std_tcp_accept_handler(basic_conn_t *bc) {
    return make_std_tcp_accept(bc);
}


const struct role_propt role_propts[] = {
    {LD_TCP_CLIENT, NULL, init_std_tcp_conn_handler},
    {LD_TCP_SERVER, make_std_tcp_server, init_std_tcp_accept_handler},
    {0, 0, 0},
};

const struct role_propt *get_role_propt(int s_r) {
    for (int i = 0; role_propts[i].s_r != 0; i++) {
        if (role_propts[i].s_r == s_r)
            return role_propts + i;
    }
    return NULL;
}


static int add_listen_fd(int epoll_fd, int server_fd) {
    set_fd_nonblocking(server_fd);
    struct epoll_event ev;
    int *fd_ptr = calloc(1, sizeof(int));
    memcpy(fd_ptr, &server_fd, sizeof(int));
    ev.data.ptr = fd_ptr;
    ev.events = EPOLLIN | EPOLLET;
    return core_epoll_add(epoll_fd, server_fd, &ev);
}

void server_entity_setup(uint16_t port, net_opt_t *opt) {
    const struct role_propt *rp = get_role_propt(LD_TCP_SERVER);

    opt->server_fd = rp->server_make(port);

    ABORT_ON(opt->accept_handler == NULL, "Accept handler is NULL");
    ABORT_ON(opt->server_fd == ERROR, "make_server");
    ABORT_ON((opt->epoll_fd = core_epoll_create(0, opt->epoll_fd)) == ERROR, "core_epoll_create");
    ABORT_ON(add_listen_fd(opt->epoll_fd, opt->server_fd) == ERROR, "add_listen_fd");

    log_info("SGW server successfully started.");
}

void *client_entity_setup(net_opt_t *opt) {
    void *conn = opt->init_handler(opt);
    if (!conn) return NULL;
    // pthread_create(th, NULL, net_setup, opt);
    return conn;
}

int server_shutdown(int server_fd) {
    return close(server_fd);
}

/**
 * The first udp packet
 * @param md
 * @param but
 * @return
 */
int read_first_packet(basic_conn_t *bc, int pre_fd) {
    uint8_t buf[MAX_INPUT_BUFFER_SIZE];
    ssize_t len;
    socklen_t addr_size = SADDR_STG_SIZE;
    zero(&bc->read_pkt);

    struct sockaddr_in *sock = (struct sockaddr_in *) &bc->saddr;
    struct in_addr in = sock->sin_addr;

    char str[INET_ADDRSTRLEN]; //INET_ADDRSTRLEN这个宏系统默认定义 16
    inet_ntop(AF_INET, &in, str, sizeof(str));
    int port = ntohs(sock->sin_port);

    len = recvfrom(pre_fd, buf, 1024, 0, (struct sockaddr *) &bc->saddr, &addr_size);

    in = sock->sin_addr;
    inet_ntop(AF_INET, &in, str, sizeof(str));
    port = ntohs(sock->sin_port);
    fprintf(stderr, "%s %d\n", str, port);

    if (len) {
        CLONE_TO_CHUNK(bc->read_pkt, buf, len)
        return OK;
    } else {
        return ERROR;
    }
}


static int read_packet(int fd, buffer_t *but) {
    uint8_t temp[MAX_INPUT_BUFFER_SIZE];
    ssize_t len;

    len = read(fd, temp, sizeof(temp));
    if (len > 0) {
        // log_warn("%d\n%s", len, temp);
        CLONE_TO_CHUNK(*but, temp, len)
        return OK;
    } else {
        // log_warn("%d", len);
        return ERROR;
    }
}


int request_handle(basic_conn_t *bc) {
    if (read_packet(bc->fd, &bc->read_pkt) == ERROR) return ERROR;
    bc->opt->recv_handler(bc);

    return OK;
}

/**
 * Return:
 * OK: all data sent
 * AGAIN: haven't sent all data
 * ERROR: error
 */
static int write_packet(basic_conn_t *bc) {
    size_t len;
    buffer_t *b;

    while (lfqueue_size(bc->write_pkts) != 0) {
        lfqueue_get(bc->write_pkts, (void **) &b);
        if (!b) return ERROR;
        len = write(bc->fd, b->ptr, b->len);
        free_buffer(b);

        /* delay the next transmission */
        usleep(1000);

        if (!len) {
            return ERROR;
        }
    }
    return OK;
}

static int response_send_buffer(basic_conn_t *bc) {
    int status;
    status = write_packet(bc);
    if (status != OK) {
        return status;
    } else {
        bc->trans_done = TRUE;
        return OK;
    }
}


int response_handle(basic_conn_t *bc) {
    int status;

    if (bc->opt->send_handler) {
        bc->opt->send_handler(bc);
    }
    do {
        status = response_send_buffer(bc);
    } while (status == OK && bc->trans_done != TRUE);
    if (bc->trans_done) {
        // response done
        if (bc->opt->reset_conn) bc->opt->reset_conn(bc);
        net_epoll_in(bc->opt->epoll_fd, bc);
    }
    return status;
}

void *net_setup(void *args) {
    int nfds;
    int i;
    net_opt_t *net_opt = args;
    while (TRUE) {
        nfds = core_epoll_wait(net_opt->epoll_fd, epoll_events, MAX_EVENTS, 20);

        if (nfds == ERROR) {
            // if not caused by signal, cannot recover
            ERR_ON(errno != EINTR, "core_epoll_wait");
        }

        /* processing ready fd one by one */
        for (i = 0; i < nfds; i++) {
            struct epoll_event *curr_event = epoll_events + i;
            int fd = *((int *) curr_event->data.ptr);
            if (fd == net_opt->server_fd) {
                // gs_conn_accept(net_opt); /* never happened in GS */
                net_opt->accept_handler(net_opt);
            } else {
                basic_conn_t *bc = curr_event->data.ptr;
                int status;
                assert(bc != NULL);

                if (connecion_is_expired(bc, net_opt->timeout))
                    continue;

                if (curr_event->events & EPOLLIN) {
                    //recv
                    status = request_handle(bc);
                }
                if (curr_event->events & EPOLLOUT) {
                    //send
                    status = response_handle(bc);
                }

                if (status == ERROR) {
                    connecion_set_expired(bc);
                } else {
                    connecion_set_reactivated(bc);
                }
            }
        }
        server_connection_prune(net_opt);
    }
    close(net_opt->epoll_fd);
    server_shutdown(net_opt->server_fd);
}

