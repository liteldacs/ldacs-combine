//
// Created by 邹嘉旭 on 2024/4/24.
//

#include "http_server.h"

static l_err init_http_service();


ld_service_t http_service = {
    .init_service = init_http_service,
    .handle_state_chg = handle_st_chg_http,
    .handle_as_info_key_upd = handle_as_info_key_upd_http,
    .handle_as_info_upd = handle_as_info_upd_http,
    .handle_recv_user_msg = handle_user_msg_http,
};

http_server_t http_server = {
};

static void start_http_server() {
    struct event_base *hbase = http_server.http_base;
    struct evhttp *hentity = http_server.http_entity;

    if (hbase) {
        event_base_dispatch(hbase);
    }
    if (hbase)
        event_base_free(hbase);
    if (hentity)
        evhttp_free(hentity);
}

static struct bufferevent *http_bev_cb(struct event_base *base, void *arg) {
    // 设置 BEV_OPT_THREADSAFE 选项以确保线程安全
    return bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
}

static l_err init_http_service() {
    log_info("The ldacs simulator is using 'HTTP' mode. Listening on %d.", config.http_port);
    evthread_use_pthreads();


    if ((http_server.http_base = event_base_new()) == NULL) {
        log_error("Cannot init http event_base");
        return LD_ERR_INTERNAL;
    }
    if ((http_server.http_entity = evhttp_new(http_server.http_base)) == NULL) {
        log_error("Cannot init http entity");
        return LD_ERR_INTERNAL;
    }
    evhttp_set_bevcb(http_server.http_entity, http_bev_cb, NULL);
    init_http_controllers(http_server.http_entity);
    if (evhttp_bind_socket(http_server.http_entity, "0.0.0.0", config.http_port) != 0) {
        log_error("Cannot bind address and port");
    }

    evhttp_set_timeout(http_server.http_entity, config.timeout);

    start_http_server();
    return LD_OK;
}

