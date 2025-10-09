//
// Created by jiaxv on 25-9-27.
//
#include <ld_dashboard.h>
#include <ld_net.h>

#include "service/service.h"

static dashboard_obj_t dashboard_obj;
pthread_t data_th;

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
        case DASHBOARD_ACCELRATE_AS: {
            dashboard_accelerate_as_t accelerate_as;
            cJSON *data = cJSON_Parse(to_resp.data);
            unmarshel_json(data, &accelerate_as, dashboard_func_defines[DASHBOARD_ACCELRATE_AS].tmpl);
            cJSON_Delete(data);
            uint8_t multiplier = accelerate_as.multiplier;
            if (multiplier > 4 || multiplier < 1) {
                log_warn("The accelerate multiplier can not more than 4 or less than 1");
                break;
            }
            rcu_set_accelerate_multiplier(accelerate_as.multiplier);
            break;
        }
        default: {
            log_warn("Wrong Response Type");
            return LD_ERR_INTERNAL;
        }
    }

    return LD_OK;
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
    dashboard_data_send(&dashboard_obj, GS_ACCESS_AS, &(dashboard_as_info_upd_t){.UA = as_info->AS_UA, .AS_SAC = as_info->AS_SAC, .GS_SAC = as_info->AS_CURR_GS_SAC});
}

static void handle_st_chg_dashboard(lme_state_chg_t *st_chg) {
    if (config.direct) {
        if (st_chg->state != LME_OPEN) return;

        pthread_create(&data_th, NULL, send_user_data_func, NULL);
        pthread_detach(data_th);
    }
}

static void handle_register_as_dashboard(uint32_t AS_UA, double longitude, double latitude) {
    dashboard_data_send(&dashboard_obj, AS_REGISTER,
                        &(dashboard_update_coordinate_t){.UA = AS_UA, .longitude = longitude, .latitude = latitude, .is_direct = config.direct});
}

static void handle_register_gs_dashboard(uint16_t GS_TAG, double longitude, double latitude) {
    dashboard_data_send(&dashboard_obj, GS_REGISTER,
                        &(dashboard_register_gs_t){.TAG = GS_TAG, .longitude = longitude, .latitude = latitude});
}

static void handle_update_coordinates_dashboard(uint32_t AS_UA, double longitude, double latitude) {
    dashboard_data_send(&dashboard_obj, AS_UPDATE_COORDINATE,
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

    dashboard_data_send(&dashboard_obj, AS_GS_RECEIVED_MSG,
                        &(dashboard_received_msg_t){.orient = orient, .type = CONTROL_PLANE_PACKET, .sender = orient == FL ? osdu->GS_SAC : AS_UA, .receiver = orient == FL ? AS_UA : osdu->GS_SAC, .data = b64_buf});

    free_buffer(b64_buf);
}

static void handle_as_exit_dashboard(uint32_t UA) {
    dashboard_data_send(&dashboard_obj, GS_AS_EXITED,
                        &(dashboard_as_exit_t){.UA = UA});
}

static void handle_received_user_message_dashboard(user_msg_t *user_msg) {
    buffer_t *b64_buf = encode_b64_buffer(0, user_msg->msg->ptr, user_msg->msg->len);
    ld_orient orient = config.role == LD_AS ? FL : RL;

    uint32_t AS_UA = 0;
    if (config.role == LD_AS) {
        AS_UA = lme_layer_objs.lme_as_man->AS_UA;
    }else if (config.role == LD_GS){
        lme_as_man_t *as_man  = get_lme_as_enode(user_msg->AS_SAC);
        AS_UA = as_man->AS_UA;
    }
    dashboard_data_send(&dashboard_obj, AS_GS_RECEIVED_MSG,
                        &(dashboard_received_msg_t){.orient = orient, .type = USER_PLANE_PACKET, .sender = orient == FL ? user_msg->GS_SAC : AS_UA, .receiver = orient == FL ? AS_UA : user_msg->GS_SAC, .data = b64_buf});

    free_buffer(b64_buf);
}

static l_err init_dashboard_service() {
    log_info("The ldacs simulator is using 'DASHBOARD' mode. Connecting to %d.", BACKEND_PORT);

    init_dashboard(&dashboard_obj, dashboard_data_recv);

    rcu_power_on(config.role); //power on directly
    if (config.role == LD_AS) rcu_start_auth();

    while (1) {
        sleep(10000);
    }

    return LD_OK;
}
