//
// Created by 邹嘉旭 on 2024/4/21.
//


#include "layer_rcu.h"
#include "layer_interface.h"

static l_err init_path_function();

rcu_layer_obj_t rcu_layer_obj = {
    .rcu_status = RCU_CLOSED,
    .is_occupied = FALSE,
    .need_access = FALSE,
    .has_access = FALSE,
    .need_exit = FALSE,
    .accel_multiplier = 1,
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
            // rcu_layer_obj.lme_status = prim->prim_obj_typ;
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
                case LME_CTRL_MSG: {
                    if (!rcu_layer_obj.service->handle_received_ctrl_message) {
                        break;
                    }
                    rcu_layer_obj.service->handle_received_ctrl_message(prim->prim_objs);
                    break;
                }
                case LME_AS_EXIT: {
                    if (!rcu_layer_obj.service->handle_as_exit) {
                        break;
                    }
                    rcu_layer_obj.service->handle_as_exit(*(uint32_t *)prim->prim_objs);
                    break;
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
        init_path_function();
        if (rcu_layer_obj.service->handle_register_as) {
            rcu_layer_obj.service->handle_register_as(config.UA, config.start_longitude, config.start_latitude, rcu_layer_obj.path.angle);
        }
    }else if (config.role == LD_GS) {
        if (rcu_layer_obj.service->handle_register_gs) {
            rcu_layer_obj.service->handle_register_gs(config.GS_SAC, config.start_longitude, config.start_latitude);
        }
    }

    // if (config.role == LD_AS) {
    //     init_path_function();
    // }
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

    rcu_layer_obj.has_access = TRUE;
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

l_rcu_err rcu_update_key(uint16_t sac) {
    return LD_RCU_OK;
}

l_rcu_err rcu_start_stop_as() {
    rcu_layer_obj.path.is_stop = !rcu_layer_obj.path.is_stop;
    return LD_RCU_OK;
}

l_rcu_err rcu_set_accelerate_multiplier(uint8_t multiplier) {
    rcu_layer_obj.accel_multiplier = multiplier;
    return LD_RCU_OK;
}

static l_err init_single_path(path_function_t *path) {
    // 添加边界检查
    if (!path) {
        log_error("path struct is null");
        return LD_ERR_NULL;
    }

    srand(time(NULL));
    double rand_basic = ((((double) rand() / (double) RAND_MAX)) - 0.5) ;

    if (config.direct) {
        path->angle = rand_basic * 180;
    }else {
        path->angle = rand_basic * 5;
    }

    // 使用经纬度计算公式，基于起始点、方向角度和2000公里距离计算终点坐标
    double distance_km = 2000.0; // 2000公里
    double earth_radius_km = 6371.0; // 地球半径（公里）

    // 将角度转换为弧度进行计算
    double lat1_rad = path->start_position[1] * M_PI / 180.0; // 起始纬度转弧度
    double lon1_rad = path->start_position[0] * M_PI / 180.0; // 起始经度转弧度
    double bearing_rad = (path->angle + 180.0) * M_PI / 180.0; // 方位角转弧度

    // 使用球面三角公式计算终点经纬度
    double lat2_rad = asin(sin(lat1_rad) * cos(distance_km / earth_radius_km) +
                          cos(lat1_rad) * sin(distance_km / earth_radius_km) * cos(bearing_rad));

    double lon2_rad = lon1_rad + atan2(sin(bearing_rad) * sin(distance_km / earth_radius_km) * cos(lat1_rad),
                                      cos(distance_km / earth_radius_km) - sin(lat1_rad) * sin(lat2_rad));

    // 转换回角度并保存到终点坐标
    path->end_position[1] = lat2_rad * 180.0 / M_PI; // 终点纬度
    path->end_position[0] = lon2_rad * 180.0 / M_PI; // 终点经度


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
        // usleep(400000 >> rcu_layer_obj.accel_multiplier);
        usleep(400000);
        if (path->is_stop) continue;

        //更新位置
        rcu_layer_obj.service->handle_update_coordinates(config.UA, path->path_points[i][0], path->path_points[i][1]);
        path->curr_position = path->path_points[i];

        if (calculate_distance(rcu_layer_obj.path.curr_position, GS1_COORDINATE) <= GS_COVERAGE) {
            rcu_layer_obj.need_access = TRUE;
        }

        if (rcu_layer_obj.has_access && !rcu_layer_obj.need_exit && calculate_distance(rcu_layer_obj.path.curr_position, GS1_COORDINATE) > GS_COVERAGE && calculate_distance(
                rcu_layer_obj.path.curr_position, GS2_COORDINATE) > GS_COVERAGE) {
            rcu_layer_obj.need_exit = TRUE;
            rcu_power_off();
        }

        i += rcu_layer_obj.accel_multiplier;
    }
}

static l_err init_path_function() {
    path_function_t *path = &rcu_layer_obj.path;
    zero(path);
    path->start_position[0] = config.start_longitude;
    path->start_position[1] = config.start_latitude;
    path->refer_position[0] = config.refer_longitude;
    path->refer_position[1] = config.refer_latitude;

    init_single_path(path);

    pthread_create(&path->th, NULL, path_function_thread, NULL);
    pthread_detach(path->th);

    return LD_OK;
}