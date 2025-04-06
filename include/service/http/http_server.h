//
// Created by 邹嘉旭 on 2024/4/24.
//

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "http_core.h"

typedef struct http_sse_req_s {
    struct evhttp_request *sse_req;
    struct list_head lpointer; //链表节点
} http_sse_req_t;

static http_sse_req_t *init_http_sse_req(struct evhttp_request *req) {
    http_sse_req_t *sse = (http_sse_req_t *) malloc(sizeof(http_sse_req_t));
    sse->sse_req = req;
    return sse;
}

typedef struct http_server_s {
    pthread_t http_th;
    int fd;
    struct event_base *http_base;
    struct evhttp *http_entity;

    /* intervene */
    // http_sse_req_t sse_reqs;
    // struct list_head *sse_req_head;
    struct evhttp_request *sse_req;
} http_server_t;

extern http_server_t http_server;


#endif //HTTP_SERVER_H
