//
// Created by jiaxv on 25-9-27.
//
#include <ld_dashboard.h>
#include <ld_net.h>
#include <crypto/key.h>
#include <crypto/secure_core.h>

#include "service/service.h"

static dashboard_obj_t dashboard_obj;
pthread_t data_th;

static l_err init_dashboard_service();
static void handle_as_info_key_upd_dashboard(as_info_key_upd_t *as_upd);
static void handle_as_info_upd_dashboard(as_info_upd_t *as_info);
static void handle_st_chg_dashboard(lme_state_chg_t *st_chg);
static void handle_register_as_dashboard(uint32_t AS_UA, double longitude, double latitude, double angle);
static void handle_register_gs_dashboard(uint16_t GS_TAG, double longitude, double latitude);
static void handle_update_coordinates_dashboard(uint32_t AS_UA, double longitude, double latitude);
static void handle_received_user_message_dashboard(user_msg_t *user_msg);
static void handle_received_control_message_dashboard(orient_sdu_t *osdu);
static void handle_as_exit_dashboard(uint32_t);
static void handle_query_keys_dashboard();


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
        case DASHBOARD_QUERY_KEYS: {
            handle_query_keys_dashboard();
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

static void handle_register_as_dashboard(uint32_t AS_UA, double longitude, double latitude, double angle) {
    dashboard_data_send(&dashboard_obj, AS_REGISTER,
                        &(dashboard_update_coordinate_t){.UA = AS_UA, .longitude = longitude, .latitude = latitude, .is_direct = config.direct, .angle = angle});
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


#pragma pack(1)
typedef struct ld_query_key_s{
    buffer_t *id;
    buffer_t *type;
    buffer_t *owner1;
    buffer_t *owner2;
    buffer_t *key_cipher;
    uint32_t key_len;
    buffer_t *key_state;
    buffer_t *create_time;
    uint32_t update_cycle;
    uint32_t kek_len;
    buffer_t * kek_cipher;
    buffer_t * iv;
    uint32_t iv_len;
    buffer_t *check_algo;
    uint32_t check_len;
    buffer_t *check_value;
    uint32_t update_count;
}ld_query_key_t;
#pragma pack()

static json_tmpl_t query_key_tmpl[] = {
    {cJSON_String, sizeof(buffer_t *), "ID", "ID", NULL},
    {cJSON_String, sizeof(buffer_t *), "type", "type", NULL},
    {cJSON_String, sizeof(buffer_t *), "owner1", "owner1", NULL},
    {cJSON_String, sizeof(buffer_t *), "owner2", "owner2", NULL},
    {cJSON_String, sizeof(buffer_t *), "keyCipher", "keyCipher", NULL},
    {cJSON_Number, sizeof(uint32_t), "keyLen", "keyLen", NULL},
    {cJSON_String, sizeof(buffer_t *), "keyState", "keyState", NULL},
    {cJSON_String, sizeof(buffer_t *), "createTime", "createTime", NULL},
    {cJSON_Number, sizeof(uint32_t), "updateCycle", "updateCycle", NULL},
    {cJSON_Number, sizeof(uint32_t), "kekLen", "kekLen", NULL},
    {cJSON_String, sizeof(buffer_t *), "kekCipher", "kekCipher", NULL},
    {cJSON_String, sizeof(buffer_t *), "iv", "iv", NULL},
    {cJSON_Number, sizeof(uint32_t), "ivLen", "ivLen", NULL},
    {cJSON_String, sizeof(buffer_t *), "checkAlgo", "checkAlgo", NULL},
    {cJSON_Number, sizeof(uint32_t), "checkLen", "checkLen", NULL},
    {cJSON_String, sizeof(buffer_t *), "checkValue", "checkValue", NULL},
    {cJSON_Number, sizeof(uint32_t), "updateCount", "updateCount", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

static json_tmpl_desc_t query_key_tmpl_desc = {
    .desc = "QUERY KEY",
    .tmpl = query_key_tmpl,
    .size = sizeof(ld_query_key_t)
};

static ld_query_key_t *init_keystore() {
    ld_query_key_t *key = malloc(sizeof(ld_query_key_t));
    if (key == NULL) {
        return NULL;
    }

    key->id = init_buffer_unptr();
    key->type = init_buffer_unptr();
    key->owner1 = init_buffer_unptr();
    key->owner2 = init_buffer_unptr();
    key->key_cipher = init_buffer_unptr();
    key->key_state = init_buffer_unptr();
    key->create_time = init_buffer_unptr();
    key->kek_cipher = init_buffer_unptr();
    key->iv = init_buffer_unptr();
    key->check_algo = init_buffer_unptr();
    key->check_value = init_buffer_unptr();

    key->key_len = 0;
    key->update_cycle = 0;
    key->kek_len = 0;
    key->iv_len = 0;
    key->check_len = 0;
    key->update_count = 0;

    return key; // 成功
}

static void free_keystore(ld_query_key_t *key) {
    if (!key)   return;

    if (key->id)    free_buffer(key->id);
    if (key->type)  free_buffer(key->type);
    if (key->owner1) free_buffer(key->owner1);
    if (key->owner2) free_buffer(key->owner2);
    if (key->key_cipher) free_buffer(key->key_cipher);
    if (key->key_state) free_buffer(key->key_state);
    if (key->create_time) free_buffer(key->create_time);
    if (key->kek_cipher) free_buffer(key->kek_cipher);
    if (key->iv) free_buffer(key->iv);
    if (key->check_algo) free_buffer(key->check_algo);
    if (key->check_value) free_buffer(key->check_value);

    free(key);
}

static void handle_query_keys_dashboard() {
    sqlite3 *db;
    int rc;
    char *db_name = NULL;
    char *query_sql = NULL;

    switch (config.role) {
        case LD_AS: {
            UA_STR(ua_as);
            db_name = get_db_name(LD_AS, get_ua_str(config.UA, ua_as));
            query_sql = "SELECT * FROM as_keystore ORDER BY creatime DESC;";
            break;
        }
        case LD_GS: {
            UA_STR(ua_gs);
            db_name = get_db_name(LD_GS, get_ua_str(config.GS_SAC, ua_gs));
            query_sql = "SELECT * FROM gs_keystore ORDER BY creatime DESC;";
            break;
        }
        default: {
            log_warn("Wrong role for query key service");
            return;
        }
    }
    if (!db_name || !query_sql) {
        log_error("DB PATH is null");
        return;
    }

    dashboard_query_keys_t query_keys = {0};
    // 打开数据库连接
    rc = sqlite3_open(db_name, &db);

    if (rc != SQLITE_OK) {
        log_error("Cannot open database: %s\n", sqlite3_errmsg(db));
        return ;
    }

    // 执行 SELECT * 查询
    sqlite3_stmt *stmt;

    rc = sqlite3_prepare_v2(db, query_sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        // 遍历结果集
        while (sqlite3_step(stmt) == SQLITE_ROW && query_keys.key_count < KEY_MAX) {
            ld_query_key_t *key = init_keystore();
            CLONE_BY_CHARS(*key->id, (const uint8_t*)sqlite3_column_text(stmt, 0))
            CLONE_BY_CHARS(*key->type, (const uint8_t*)sqlite3_column_text(stmt, 1))
            CLONE_BY_CHARS(*key->owner1, (const uint8_t*)sqlite3_column_text(stmt, 2))
            CLONE_BY_CHARS(*key->owner2, (const uint8_t*)sqlite3_column_text(stmt, 3))
            CLONE_BY_CHARS(*key->key_cipher, (const uint8_t*)sqlite3_column_text(stmt, 4))
            key->key_len = sqlite3_column_int(stmt, 5);
            CLONE_BY_CHARS(*key->key_state, (const uint8_t*)sqlite3_column_text(stmt, 6))
            CLONE_BY_CHARS(*key->create_time, (const uint8_t*)sqlite3_column_text(stmt, 7))
            key->update_cycle = sqlite3_column_int(stmt, 8);
            key->kek_len = sqlite3_column_int(stmt, 9);
            CLONE_BY_CHARS(*key->kek_cipher, (const uint8_t*)sqlite3_column_text(stmt, 10))
            CLONE_BY_CHARS(*key->iv, (const uint8_t*)sqlite3_column_text(stmt, 11))
            key->iv_len = sqlite3_column_int(stmt, 12);
            CLONE_BY_CHARS(*key->check_algo, (const uint8_t*)sqlite3_column_text(stmt, 13))
            key->check_len = sqlite3_column_int(stmt, 14);
            CLONE_BY_CHARS(*key->check_value, (const uint8_t*)sqlite3_column_text(stmt, 15))
            key->update_count = sqlite3_column_int(stmt, 16);

            query_keys.j_keys[query_keys.key_count++] = marshel_json(key, &query_key_tmpl_desc);

            free_keystore(key);
        }

        dashboard_data_send(&dashboard_obj, AS_GS_QUERY_KEYS, &query_keys);
    } else {
        log_error("Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }

    // 清理资源
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}
