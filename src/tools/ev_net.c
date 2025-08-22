//
// Created by jiaxv on 25-8-22.
//

#include "ev_net.h"
// 服务端读取回调
static void server_read_cb(struct bufferevent *bev, void *ctx) {
    ev_server_context_t *server = (ev_server_context_t *)ctx;
    struct evbuffer *input = bufferevent_get_input(bev);
    char buffer[BUFFER_SIZE];

    while (evbuffer_get_length(input)) {
        int n = evbuffer_remove(input, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            log_info("[%s] 收到数据: %s", server->name, buffer);

            if (server->recv_handler) {
                server->recv_handler(server->arg);
            }
        }
    }
}

// 客户端读取回调
static void client_read_cb(struct bufferevent *bev, void *ctx) {
    ev_client_context_t *client = (ev_client_context_t *)ctx;
    struct evbuffer *input = bufferevent_get_input(bev);
    char buffer[BUFFER_SIZE];

    while (evbuffer_get_length(input)) {
        int n = evbuffer_remove(input, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            if (client->recv_handler) {
                client->recv_handler(client->arg);
            }
            // printf("[%s] 收到回复: %s", client->name, buffer);
        }
    }
}


// 服务端事件回调
static void server_event_cb(struct bufferevent *bev, short events, void *ctx) {
    ev_server_context_t *server = (ev_server_context_t *)ctx;

    if (events & BEV_EVENT_ERROR) {
        log_error("[%s] 连接错误\n", server->name);
    }
    if (events & BEV_EVENT_EOF) {
        log_info("[%s] 客户端断开连接\n", server->name);
    }

    bufferevent_free(bev);
}

static void client_event_cb(struct bufferevent *bev, short events, void *ctx) {
    ev_client_context_t *client = (ev_client_context_t *)ctx;

    if (events & BEV_EVENT_CONNECTED) {
        log_info("[%s] 成功连接到 %s\n", client->name, client->peer_ip);
    } else if (events & BEV_EVENT_ERROR) {
        log_error("[%s] 连接错误\n", client->name);
    } else if (events & BEV_EVENT_EOF) {
        log_warn("[%s] 连接关闭\n", client->name);
    }
}



// 服务端接受连接回调
static void accept_conn_cb(struct evconnlistener *listener, evutil_socket_t fd,
                          struct sockaddr *address, int socklen, void *ctx) {
    ev_server_context_t *server = (ev_server_context_t *)ctx;
    struct event_base *base = evconnlistener_get_base(listener);
    struct bufferevent *bev;

    // 创建bufferevent
    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (!bev) {
        log_warn("[%s] 创建bufferevent失败\n", server->name);
        close(fd);
        return;
    }

    // 设置回调
    bufferevent_setcb(bev, server_read_cb, NULL, server_event_cb, server);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

static void *run_server(void *args) {
    ev_server_context_t *server = args;
    // 运行事件循环
    event_base_dispatch(server->base);

    // 清理
    evconnlistener_free(server->listener);
    event_base_free(server->base);

    return NULL;
}

static void *run_client(void *args) {
    ev_client_context_t *client = args;
    // 运行事件循环
    event_base_dispatch(client->base);

    if (client->bev) {
        bufferevent_free(client->bev);
    }
    event_base_free(client->base);

    return NULL;
}


l_err setup_server(ev_server_context_t *server) {
    evthread_use_pthreads();
    server->base = event_base_new();
    if (!server->base) {
        log_info("[%s] 创建event_base失败\n", server->name);
        return LD_ERR_INTERNAL;
    }

    struct sockaddr_in sin = {0};

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(server->port);

    // 创建监听器
    server->listener = evconnlistener_new_bind(
        server->base, accept_conn_cb, server,
        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
        -1, (struct sockaddr*)&sin, sizeof(sin));

    if (!server->listener) {
        log_error("[%s] 创建监听器失败\n", server->name);
        event_base_free(server->base);
        return LD_ERR_INTERNAL;
    }

    log_info("Setting up server... listening on port: %d", server->port);

    pthread_create(&server->th, NULL, run_server, server);
    pthread_detach(server->th);
    return LD_OK;
}

l_err setup_client(ev_client_context_t *client) {
    evthread_use_pthreads();
    client->base = event_base_new();
    if (!client->base) {
        log_error("[%s] 创建event_base失败\n", client->name);
        return LD_ERR_INTERNAL;
    }

    // 创建 socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        log_error("[%s] 创建socket失败\n", client->name);
        event_base_free(client->base);
        return LD_ERR_INTERNAL;
    }

    // 设置 socket 选项，允许地址重用
    int on = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    // 绑定本地地址和端口
    struct sockaddr_in local_sin = {0};
    local_sin.sin_family = AF_INET;
    local_sin.sin_addr.s_addr = INADDR_ANY;  // 或者指定具体的本地IP
    local_sin.sin_port = htons(client->local_port);  // 使用固定的本地端口

    if (bind(sock, (struct sockaddr *)&local_sin, sizeof(local_sin)) < 0) {
        log_error("[%s] 绑定本地端口 %d 失败: %s\n",
                  client->name, client->local_port, strerror(errno));
        close(sock);
        event_base_free(client->base);
        return LD_ERR_INTERNAL;
    }

    // 设置为非阻塞模式
    evutil_make_socket_nonblocking(sock);


    client->bev = bufferevent_socket_new(client->base, sock, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(client->bev, client_read_cb, NULL, client_event_cb, client);
    bufferevent_enable(client->bev, EV_READ | EV_WRITE);

    struct sockaddr_in sin = {0};
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(client->peer_ip);
    sin.sin_port = htons(client->peer_port);

    if (bufferevent_socket_connect(client->bev,
        (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        log_error("[%s] 连接服务端\n", client->name);
        bufferevent_free(client->bev);
        client->bev = NULL;
        return LD_ERR_INTERNAL;
    }

    log_info("Setting up client... listening on port: %d", client->local_port);

    pthread_create(&client->th, NULL, run_client, client);
    pthread_detach(client->th);

    return LD_OK;
}
