//
// Created by jiaxv on 25-9-27.
//
#include "dashboard.h"
#include <ld_net.h>

typedef struct dashboad_obj_s {
    pthread_t conn_th;
    net_ctx_t net_ctx;
    basic_conn_t *conn;
}dashboad_obj_t;

static dashboad_obj_t dashboad_obj = {
    .conn_th = 0,
};

static l_err init_dashboard_service();

ld_service_t dashboard_service = {
    .init_service = init_dashboard_service,
};

static void *dashboard_conn_connect(net_ctx_t *ctx, char *remote_addr, int remote_port, int local_port) {
    basic_conn_t *bc = malloc(sizeof(basic_conn_t));

    bc->remote_addr = strdup(remote_addr);
    bc->remote_port = remote_port;
    bc->local_port = local_port;

    if (init_basic_conn_client(bc, ctx, LD_TCP_CLIENT) == FALSE) {
        return NULL;
    }

    return bc;
}

static l_err dashboard_data_recv(basic_conn_t *bc) {
    log_warn("!!!!!!!!~~~~~~");
    return LD_OK;
}

static l_err dashboard_data_send(DASHBOARD_FUNCTION func_e, void *data) {

    char *data_str = NULL;
    if (get_json_str(data, dashboard_func_defines[func_e].tmpl, &data_str) != LD_OK) {
        log_warn("Cannot generate JSON string");
        return LD_ERR_INTERNAL;
    }

    dashboard_data_t to_resp = {
        .func = (uint8_t)func_e,
        .data = data_str,
    };

    char *root_s;
    get_json_str(&to_resp, &dashboard_data_tmpl_desc, &root_s);

    buffer_t *to_send = init_buffer_unptr();
    CLONE_TO_CHUNK(*to_send, root_s, strlen(root_s));

    // cn_log_buf(LOG_WARN, meta->net_ele->element_tag, meta->interface_type, "SEND", to_send->ptr, to_send->len);

    if (dashboad_obj.net_ctx.send_handler(dashboad_obj.conn, to_send, NULL, NULL) != LD_OK) {
        log_error("Send Dashboard data Failed!");
        return LD_ERR_INTERNAL;
    }

    free_buffer(to_send);
    free(data_str);
    free(root_s);

    return LD_OK;
}

static void dashboard_conn_close(basic_conn_t *bc) {
    if (!bc) return;
    // delete_conn_enode_by_connptr(gs_conn, NULL);
    free(bc);
    log_warn("Closing connection!");
}

static l_err init_dashboard_service() {
    // log_info("The ldacs simulator is using 'DASHBOARD' mode. Connecting to %s:%d.", BACKEND_IP"\0", BACKEND_PORT);
    log_info("The ldacs simulator is using 'DASHBOARD' mode. Connecting to %d.", BACKEND_PORT);
    dashboad_obj.net_ctx = (net_ctx_t){
        .conn_handler = dashboard_conn_connect,
        .recv_handler = dashboard_data_recv,
        .close_handler = dashboard_conn_close,
        .send_handler = defalut_send_pkt,
        .epoll_fd = core_epoll_create(0, -1),
    };


    dashboad_obj.conn = client_entity_setup(&dashboad_obj.net_ctx, BACKEND_IP, BACKEND_PORT, config.dashboard_port);
    pthread_create(&dashboad_obj.conn_th, NULL, net_setup, &dashboad_obj.net_ctx);

    rcu_power_on(config.role); //power on directly
    if (config.role == LD_AS) rcu_start_auth();

    pthread_join(dashboad_obj.conn_th, NULL);

    // net_setup(&dashboad_obj.net_ctx);
    return LD_OK;
}
