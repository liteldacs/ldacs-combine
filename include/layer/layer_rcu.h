//
// Created by 邹嘉旭 on 2024/4/21.
//

#ifndef LAYER_RCU_H
#define LAYER_RCU_H

#include "ldacs_lme.h"

typedef struct lme_state_chg_s lme_state_chg_t;
typedef struct as_info_key_upd_s as_info_key_upd_t;
typedef struct as_info_upd_s as_info_upd_t;
typedef struct user_msg_s user_msg_t;

typedef struct ld_service_s {
    l_err (*init_service)();

    void (*handle_state_chg)(lme_state_chg_t *);

    void (*handle_as_info_key_upd)(as_info_key_upd_t *);

    void (*handle_as_info_upd)(as_info_upd_t *);

    void (*handle_register_as)(uint32_t, double, double);

    void (*handle_register_gs)(uint16_t, double, double);

    void (*handle_update_coordinates)(uint32_t, double, double);

    void (*handle_recv_user_msg)(user_msg_t *);

    void (*handle_received_ctrl_message)(orient_sdu_t *);

    void (*handle_as_exit)(uint32_t);
} ld_service_t;

typedef enum RCU_RET_E {
    LD_RCU_OK = 0,
    LD_RCU_FAILED,

    //for opendevice
    LD_RCU_ALREADY_IN_STATE,
    LD_RCU_OCCUPIED,
} l_rcu_err;

enum RCU_STATUS_E {
    RCU_OPEN = 0,
    RCU_CLOSED,
};

#define GEN_POINTS 20

typedef struct path_function_s {
    double start_position[2];
    double refer_position[2];
    double end_position[2];
    double *curr_position;
    double path_points[GEN_POINTS][2];
    pthread_t th;
    bool is_stop;
}path_function_t;

typedef struct rcu_layer_obj_s {
    enum RCU_STATUS_E rcu_status;
    // enum ELE_TYP lme_status;
    bool is_occupied;
    ld_service_t *service;
    path_function_t  path;
    bool need_access;
    bool has_access;
    bool need_exit;
} rcu_layer_obj_t;

extern rcu_layer_obj_t rcu_layer_obj;

typedef struct handover_opt_s {
    uint32_t UA;
    uint32_t GST_SAC;
}handover_opt_t;

#pragma pack(1)

struct lme_state_chg_s {
    uint32_t ua;
    uint16_t sac;
    uint16_t state;
};

struct as_info_key_upd_s {
    uint32_t ua;
    buffer_t *key;
    uint64_t value;
};


struct as_info_upd_s {
    uint32_t AS_UA;
    uint16_t AS_SAC;
    uint16_t AS_CURR_GS_SAC;

    uint8_t AUTHC_MACLEN,
            AUTHC_AUTH_ID,
            AUTHC_ENC_ID,
            AUTHC_KLEN;

    uint16_t CO;
    uint8_t RPSO,
            NRPS;
};

struct user_msg_s {
    uint16_t AS_SAC;
    uint16_t GS_SAC;
    buffer_t *msg;
};

#pragma pack()


extern json_tmpl_t sse_state_tmpl[];
extern json_tmpl_t as_info_key_upd_tmpl[];
extern json_tmpl_t as_info_upd_tmpl[];
extern json_tmpl_t user_msg_tmpl[];

static json_tmpl_desc_t sse_state_tmpl_desc = {"SSE_STATE", sse_state_tmpl, sizeof(lme_state_chg_t)};
static json_tmpl_desc_t as_info_key_upd_tmpl_desc = {
    .desc = "AS INFO KEY",
    .tmpl = as_info_key_upd_tmpl,
    .size = sizeof(as_info_key_upd_t)
};
static json_tmpl_desc_t as_info_upd_tmpl_desc = {
    .desc = "AS_INFO",
    .tmpl = as_info_upd_tmpl,
    .size = sizeof(as_info_upd_t)
};
static json_tmpl_desc_t user_msg_tmpl_desc = {
    .desc = "USER_MSG",
    .tmpl = user_msg_tmpl,
    .size = sizeof(user_msg_t)
};

void L_SAPC_cb(ld_prim_t *prim);

void L_SAPT_cb(ld_prim_t *prim);

void init_rcu(ld_service_t *service);

void stop_rcu();

l_rcu_err rcu_power_on(uint8_t role);

l_rcu_err rcu_power_off();

l_rcu_err rcu_start_auth();

l_rcu_err rcu_handover(uint32_t UA, uint16_t GST_SAC);

enum RCU_STATUS_E rcu_get_rcu_state();

bool rcu_is_occupied();

l_rcu_err rcu_change_occupied(bool to_change);

l_rcu_err rcu_update_key(uint16_t sac);

l_rcu_err rcu_start_stop_as();

#endif //LAYER_RCU_H
