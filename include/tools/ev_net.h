//
// Created by jiaxv on 25-8-22.
//

#ifndef EV_NET_H
#define EV_NET_H
#include <ld_log.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>

#define BUFFER_SIZE 4096


typedef l_err (*recv_handler)(void *);

typedef struct ev_server_context_s {
    char *name;
    uint16_t port;
    void *arg;
    struct event_base *base;
    struct evconnlistener *listener;
    pthread_t thread;
    recv_handler recv_handler;
    pthread_t th;
}ev_server_context_t;

// 客户端数据结构
typedef struct ev_client_context_s {
    char *name;

    char *peer_ip;
    uint16_t peer_port;
    uint16_t local_port;
    void *arg;
    struct event_base *base;
    struct bufferevent *bev;
    recv_handler recv_handler;
    pthread_t th;
} ev_client_context_t;


l_err setup_server(ev_server_context_t *server);

l_err setup_client(ev_client_context_t *client);


#endif //EV_NET_H
