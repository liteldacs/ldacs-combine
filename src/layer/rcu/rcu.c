//
// Created by 邹嘉旭 on 2024/4/21.
//


#include "layer_rcu.h"
#include "layer_interface.h"

static l_err init_path_function();

rcu_layer_obj_t rcu_layer_obj = {
    .rcu_status = RCU_CLOSED,
    .is_occupied = FALSE,
};


static void powering_on() {
    // make_phy_layer(PHY_SIM_JSON);
    make_mac_layer();
    make_lme_layer();
    make_phy_layer(PHY_SIM_JSON);
    make_dls_layer();
    make_snp_layer();
}

void init_rcu(ld_service_t *service) {
    register_int_handler(service->handle_recv_user_msg);
    rcu_layer_obj.service = service;
    rcu_layer_obj.service->init_service();
}

void stop_rcu() {
    rcu_power_off();
}

void destory_rcu() {
}

void L_SAPC_cb(ld_prim_t *prim) {
    switch (prim->prim_seq) {
        case LME_OPEN_REQ: {
            rcu_layer_obj.rcu_status = !prim->prim_err ? RCU_OPEN : RCU_CLOSED;
            break;
        }
        case LME_STATE_IND: {
            rcu_layer_obj.lme_status = prim->prim_obj_typ;
            switch (prim->prim_obj_typ) {
                case LME_STATE_CHANGE: {
                    passert(rcu_layer_obj.service->handle_state_chg != NULL);
                    rcu_layer_obj.service->handle_state_chg(prim->prim_objs);
                    break;
                }
                case LME_AS_KEY_UPDATE: {
                    passert(rcu_layer_obj.service->handle_as_info_key_upd != NULL);
                    rcu_layer_obj.service->handle_as_info_key_upd(prim->prim_objs);
                    break;
                }
                case LME_AS_UPDATE: {
                    passert(rcu_layer_obj.service->handle_as_info_upd != NULL);
                    lme_as_man_t *as_man = prim->prim_objs;
                    rcu_layer_obj.service->handle_as_info_upd(&(as_info_upd_t){
                        .AS_UA = as_man->AS_UA,
                        .AS_SAC = as_man->AS_SAC,
                        .AS_CURR_GS_SAC = as_man->AS_CURR_GS_SAC,
                        .CO = as_man->CO.co[0],
                        .RPSO = as_man->RPSO,
                        .NRPS = as_man->NRPS,
                    });
                    break;
                }
                default: {
                    break;
                }
            }
            break;
        }
        default: {
            break;
        }
    }
}

void L_SAPT_cb(ld_prim_t *prim) {
}

bool rcu_is_occupied() {
    return rcu_layer_obj.is_occupied ? LD_RCU_OCCUPIED : LD_RCU_OK;
}

l_rcu_err rcu_change_occupied(bool to_change) {
    rcu_layer_obj.is_occupied = to_change;

    return LD_RCU_OK;
}

l_rcu_err rcu_power_on(uint8_t role) {
    // if rcu startus is OPEN, return derictly
    if (rcu_layer_obj.rcu_status == RCU_OPEN) return LD_RCU_ALREADY_IN_STATE;
    powering_on();


    if (config.role == LD_AS) {
        // 注册as
        if (rcu_layer_obj.service->handle_register_as) {
            rcu_layer_obj.service->handle_register_as(config.UA, config.start_longitude, config.start_latitude);
        }
        init_path_function();
    }else if (config.role == LD_GS) {
        if (rcu_layer_obj.service->handle_register_gs) {
            rcu_layer_obj.service->handle_register_gs(config.GS_SAC, config.start_longitude, config.start_latitude);
        }
    }
    if (preempt_prim(&LME_OPEN_REQ_PRIM, RC_TYP_OPEN, NULL, NULL, 0, 0) || rcu_layer_obj.rcu_status != RCU_OPEN) {
        log_error("Can not open Stack correctly");
        return LD_RCU_FAILED;
    }

    log_info("LDACS Stack power on correctly!");
    return LD_RCU_OK;
}


l_rcu_err rcu_power_off() {
    if (rcu_layer_obj.rcu_status == RCU_CLOSED) return LD_RCU_ALREADY_IN_STATE;

    if (preempt_prim(&LME_OPEN_REQ_PRIM, RC_TYP_CLOSE, NULL, NULL, 0, 0) != LD_OK) {
        return LD_RCU_FAILED;
    }
    return LD_RCU_OK;
}

l_rcu_err rcu_start_auth() {
    if (preempt_prim(&LME_AUTH_REQ_PRIM, E_TYP_ANY, NULL, NULL, 0, 0)) {
        log_error("Can not start Auth correctly");
        return LD_RCU_FAILED;
    }

    log_info("LDACS Stack authentication correctly!");
    return LD_RCU_OK;
}


l_rcu_err rcu_handover(uint32_t UA, uint16_t GST_SAC) {
    log_warn("========== Start Handover ==========");

    handover_opt_t opt = {UA, GST_SAC};
    if (preempt_prim(&LME_CONF_REQ_PRIM, RC_TYP_HANDOVER, &opt, NULL, 0, 0)) {
        log_error("Can not start Auth correctly");
        return LD_RCU_FAILED;
    }
    return LD_RCU_OK;
}

enum RCU_STATUS_E rcu_get_rcu_state() {
    if (config.role == LD_AS && rcu_layer_obj.rcu_status == RCU_OPEN) {
        lme_as_man_t *as_man = lme_layer_objs.lme_as_man;
        rcu_layer_obj.service->handle_as_info_upd(&(as_info_upd_t){
            .AS_UA = as_man->AS_UA,
            .AS_SAC = as_man->AS_SAC,
            .AS_CURR_GS_SAC = as_man->AS_CURR_GS_SAC,
            .CO = as_man->CO.co[0],
            .RPSO = as_man->RPSO,
            .NRPS = as_man->NRPS,
        });
    }
    return rcu_layer_obj.rcu_status;
}

enum ELE_TYP rcu_get_lme_state() {
    return rcu_layer_obj.lme_status;
}

l_rcu_err rcu_update_key(uint16_t sac) {
    return LD_RCU_OK;
}

l_rcu_err rcu_start_stop_as() {
    rcu_layer_obj.path.is_stop = !rcu_layer_obj.path.is_stop;
    return LD_RCU_OK;
}

static l_err init_path(path_function_t *path) {
    // 添加边界检查
    if (!path) {
        log_error("path struct is null");
        return LD_ERR_NULL;
    }

    srand(time(NULL));
    double angle = (((double)rand() / (double)RAND_MAX) - 0.5) * 4;

    path->end_position[0] = path->start_position[0] + (path->refer_position[0] - path->start_position[0]) *2 + angle;
    path->end_position[1] = path->start_position[1] + (path->refer_position[1] - path->start_position[1]) *2 + angle;


    for (int i = 0; i < GEN_POINTS; i++) {
        double ratio = (double)i / (double)(GEN_POINTS);
        path->path_points[i][0] = path->start_position[0] + (path->end_position[0] - path->start_position[0]) * ratio;
        path->path_points[i][1] = path->start_position[1] + (path->end_position[1] - path->start_position[1]) * ratio;
    }

    return LD_OK;
}

static void *path_function_thread(void *arg) {
    path_function_t *path = &rcu_layer_obj.path;
    int i = 0;
    if (!rcu_layer_obj.service->handle_update_coordinates) return NULL;
    while (1) {
        if (i >= GEN_POINTS) break;
        sleep(1);
        if (path->is_stop) continue;

        //更新位置
        rcu_layer_obj.service->handle_update_coordinates(config.UA, path->path_points[i][0], path->path_points[i][1]);
        path->curr_position = path->path_points[i];

        // double ret = calculate_distance(path->path_points[i], (double[2]){117, 36.5});
        // log_warn("~~ %.6f", ret);

        i++;
    }
}

static l_err init_path_function() {
    path_function_t *path = &rcu_layer_obj.path;
    zero(path);
    path->start_position[0] = config.start_longitude;
    path->start_position[1] = config.start_latitude;
    path->refer_position[0] = config.refer_longitude;
    path->refer_position[1] = config.refer_latitude;

    init_path(path);

    pthread_create(&path->th, NULL, path_function_thread, NULL);
    pthread_detach(path->th);

    return LD_OK;
}