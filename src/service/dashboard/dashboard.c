//
// Created by jiaxv on 25-9-27.
//
#include "dashboard.h"
#include <ld_net.h>

#include "service/service.h"

typedef struct dashboad_obj_s {
    pthread_t conn_th;
    net_ctx_t net_ctx;
    basic_conn_t *conn;
}dashboad_obj_t;

static dashboad_obj_t dashboad_obj = {
    .conn_th = 0,
};

static l_err init_dashboard_service();
static void handle_as_info_key_upd_dashboard(as_info_key_upd_t *as_upd);
static void handle_as_info_upd_dashboard(as_info_upd_t *as_info);
static void handle_st_chg_dashboard(lme_state_chg_t *st_chg);
static void handle_register_as_dashboard(uint32_t AS_UA, double longitude, double latitude);
static void handle_register_gs_dashboard(uint16_t GS_TAG, double longitude, double latitude);
static void handle_update_coordinates_dashboard(uint32_t AS_UA, double longitude, double latitude);
static void handle_received_user_message_dashboard(user_msg_t *user_msg);
static void handle_received_control_message_dashboard(orient_sdu_t *osdu);
static void handle_as_exit_dashboard(uint32_t);


ld_service_t dashboard_service = {
    .init_service = init_dashboard_service,
    .handle_as_info_key_upd = handle_as_info_key_upd_dashboard,
    .handle_as_info_upd = handle_as_info_upd_dashboard,
    .handle_state_chg = handle_st_chg_dashboard,
    .handle_register_as = handle_register_as_dashboard,
    .handle_register_gs = handle_register_gs_dashboard,
    .handle_update_coordinates = handle_update_coordinates_dashboard,
    .handle_received_ctrl_message = handle_received_control_message_dashboard,
    .handle_recv_user_msg = handle_received_user_message_dashboard,
    .handle_as_exit = handle_as_exit_dashboard,
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
    if (!bc->read_pkt) {
        log_warn("Read pkt is null");
        return LD_ERR_NULL;
    }

    dashboard_data_t to_resp;

    cJSON *root = cJSON_Parse((const char *)bc->read_pkt->ptr);
    unmarshel_json(root, &to_resp, &dashboard_data_tmpl_desc);
    cJSON_Delete(root);

    switch (to_resp.type) {
        case DASHBOARD_START_STOP_AS: {
            rcu_start_stop_as();
            break;
        }
        case DASHBOARD_SWITCH_AS: {
            dashboard_switch_as_t switch_as;
            cJSON *data = cJSON_Parse(to_resp.data);
            unmarshel_json(data, &switch_as, dashboard_func_defines[DASHBOARD_SWITCH_AS].tmpl);
            cJSON_Delete(data);

            rcu_handover(switch_as.UA, switch_as.GST_SAC);
            break;
        }
        case DASHBOARD_SEND_SINGLE_DATA: {
            send_singal_data();
            break;
        }
        case DASHBOARD_SEND_MULTI_DATA: {
            send_multi_datas();
            break;
        }
        default: {
            log_warn("Wrong Response Type");
            return LD_ERR_INTERNAL;
        }
    }

    return LD_OK;
}

static l_err dashboard_data_send(DASHBOARD_FUNCTION func_e, void *data) {

    char *data_str = NULL;
    if (get_json_str(data, dashboard_func_defines[func_e].tmpl, &data_str) != LD_OK) {
        log_warn("Cannot generate JSON string");
        return LD_ERR_INTERNAL;
    }

    dashboard_data_t to_resp = {
        .type = (uint8_t)func_e,
        .data = data_str,
    };

    char *root_s;
    get_json_str(&to_resp, &dashboard_data_tmpl_desc, &root_s);

    buffer_t *to_send = init_buffer_unptr();
    CLONE_TO_CHUNK(*to_send, root_s, strlen(root_s));

    // log_fatal("%s %d", root_s, strlen(root_s));
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
    free(bc);
    log_warn("Closing connection!");
}

static void handle_as_info_key_upd_dashboard(as_info_key_upd_t *as_upd) {
    const char *tag = "AS_SAC";
    // if (as_upd->key->len >= strlen(tag) && !memcmp(as_upd->key->ptr, tag, strlen(tag))) {
    //     // terminal_obj.AS_SAC = as_upd->value;
    // }
}

static void handle_as_info_upd_dashboard(as_info_upd_t *as_info) {

    // 对于dashboard,AS的注册过程已经在RCU_POWERON时完成，不需要再update了
    if (config.role == LD_AS)   return;
    dashboard_data_send(GS_ACCESS_AS, as_info);
}

static void handle_st_chg_dashboard(lme_state_chg_t *st_chg) {

}

static void handle_register_as_dashboard(uint32_t AS_UA, double longitude, double latitude) {
    dashboard_data_send(AS_REGISTER,
                        &(dashboard_update_coordinate_t){.UA = AS_UA, .longitude = longitude, .latitude = latitude, .is_direct = config.direct});
}

static void handle_register_gs_dashboard(uint16_t GS_TAG, double longitude, double latitude) {
    dashboard_data_send(GS_REGISTER,
                        &(dashboard_register_gs_t){.TAG = GS_TAG, .longitude = longitude, .latitude = latitude});
}

static void handle_update_coordinates_dashboard(uint32_t AS_UA, double longitude, double latitude) {
    dashboard_data_send(AS_UPDATE_COORDINATE,
                        &(dashboard_update_coordinate_t){.UA = AS_UA, .longitude = longitude, .latitude = latitude, .is_direct = config.direct});
}

static void handle_received_control_message_dashboard(orient_sdu_t *osdu) {
    buffer_t *b64_buf = encode_b64_buffer(0, osdu->buf->ptr, osdu->buf->len);
    ld_orient orient = config.role == LD_AS ? FL : RL;

    uint32_t AS_UA = 0;
    if (config.role == LD_AS) {
        AS_UA = lme_layer_objs.lme_as_man->AS_UA;
    }else if (config.role == LD_GS){
        lme_as_man_t *as_man  = get_lme_as_enode(osdu->AS_SAC);
        AS_UA = as_man->AS_UA;
    }

    dashboard_data_send(AS_GS_RECEIVED_MSG,
                        &(dashboard_received_msg_t){.orient = orient, .type = CONTROL_PLANE_PACKET, .sender = orient == FL ? osdu->GS_SAC : AS_UA, .receiver = orient == FL ? AS_UA : osdu->GS_SAC, .data = b64_buf});

    free_buffer(b64_buf);
}

static void handle_as_exit_dashboard(uint32_t UA) {
    dashboard_data_send(GS_AS_EXITED,
                        &(dashboard_as_exit_t){.UA = UA});
}

static void handle_received_user_message_dashboard(user_msg_t *user_msg) {
    buffer_t *b64_buf = encode_b64_buffer(0, user_msg->msg->ptr, user_msg->msg->len);
    ld_orient orient = config.role == LD_AS ? FL : RL;

    log_warn("?? %d", user_msg->GS_SAC);
    uint32_t AS_UA = 0;
    if (config.role == LD_AS) {
        AS_UA = lme_layer_objs.lme_as_man->AS_UA;
    }else if (config.role == LD_GS){
        lme_as_man_t *as_man  = get_lme_as_enode(user_msg->AS_SAC);
        AS_UA = as_man->AS_UA;
    }
    dashboard_data_send(AS_GS_RECEIVED_MSG,
                        &(dashboard_received_msg_t){.orient = orient, .type = USER_PLANE_PACKET, .sender = orient == FL ? user_msg->GS_SAC : AS_UA, .receiver = orient == FL ? AS_UA : user_msg->GS_SAC, .data = b64_buf});

    free_buffer(b64_buf);
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
