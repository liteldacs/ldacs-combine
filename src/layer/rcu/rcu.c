//
// Created by 邹嘉旭 on 2024/4/21.
//



#include "layer_rcu.h"
#include "layer_interface.h"

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

void init_rcu(ld_service_t service) {
    register_int_handler(service.handle_recv_user_msg);
    rcu_layer_obj.service = service;
    rcu_layer_obj.service.init_service();
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
                    passert(rcu_layer_obj.service.handle_state_chg != NULL);
                    rcu_layer_obj.service.handle_state_chg(prim->prim_objs);
                    break;
                }
                case LME_AS_KEY_UPDATE: {
                    passert(rcu_layer_obj.service.handle_as_info_key_upd != NULL);
                    rcu_layer_obj.service.handle_as_info_key_upd(prim->prim_objs);
                    break;
                }
                case LME_AS_UPDATE: {
                    passert(rcu_layer_obj.service.handle_as_info_upd != NULL);
                    lme_as_man_t *as_man = prim->prim_objs;
                    rcu_layer_obj.service.handle_as_info_upd(&(as_info_upd_t) {
                            .AS_UA = as_man->AS_UA,
                            .AS_SAC = as_man->AS_SAC,
                            .AS_CURR_GS_SAC = as_man->AS_CURR_GS_SAC,
//                        .AUTHC_MACLEN = as_man->AUTHC_MACLEN,
//                        .AUTHC_AUTH_ID = as_man->AUTHC_AUTH_ID,
//                        .AUTHC_ENC_ID = as_man->AUTHC_ENC_ID,
//                        .AUTHC_KLEN = as_man->AUTHC_KLEN,
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
        rcu_layer_obj.service.handle_as_info_upd(&(as_info_upd_t) {
                .AS_UA = as_man->AS_UA,
                .AS_SAC = as_man->AS_SAC,
                .AS_CURR_GS_SAC = as_man->AS_CURR_GS_SAC,
//                .AUTHC_MACLEN = as_man->AUTHC_MACLEN,
//                .AUTHC_AUTH_ID = as_man->AUTHC_AUTH_ID,
//                .AUTHC_ENC_ID = as_man->AUTHC_ENC_ID,
//                .AUTHC_KLEN = as_man->AUTHC_KLEN,
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

