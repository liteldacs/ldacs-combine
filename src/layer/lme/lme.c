//
// Created by 邹嘉旭 on 2023/12/14.
//


#include "ldacs_lme.h"

static lyr_desc_t *sn_upper_lyr[] = {
};
static lyr_desc_t *rl_upper_lyr[] = {
};

static lyr_desc_t *sn_lower_lyr[] = {
    //&snp_desc,
};

static lyr_desc_t *rl_lower_lyr[] = {
    &mac_desc,
};


lme_layer_objs_t lme_layer_objs = {
    .LME_T_CELL_RESP = 30,
    .LME_C_MAKE = 3,
    .LME_T_MAKE = 60,
    .LME_T_RLK = 10,
    .LME_T1_FLK = 3,
    .LME_T_CSAN = 60,
    .LME_C_PRLA = 10,
    .SF_NUMBER = 0,
    .MF_NUMBER = 0,

    .PROTOCOL_VER = PROTECT_VERSION,
    .MOD = MOD_CELL_SPECIFIC,
    .CMS = CMS_TYP_1,
    .EIRP = 0,

    .cc_timer = {{.it_interval = {0, MF_TIMER}, .it_value = {0, 0}}},
    .gtimer = {
        {trans_lme_bc_timer_func, NULL, TIMER_INFINITE},
        {trans_lme_cc_timer_func, NULL, 4},
        {trans_ra_cr_timer_func, NULL, TIMER_INFINITE},
    },

    .finish_status = LME_NO_STATE_FINISHED,
};

lyr_desc_t lme_desc = {
    .name = "LME_SUB_NET_FUNC",
    .lyr_funcs = NULL,
    .upper_lyr = NULL,
    .lower_lyr = sn_lower_lyr,
    .lyr_obj = &lme_layer_objs,
};

ld_prim_t LME_OPEN_REQ_PRIM = {
    .name = "LME_OPEN_REQ",
    .prim_seq = LME_OPEN_REQ,
    .SAP = {L_SAPC, NULL, NULL},
    .req_cb = {L_SAPC_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t LME_CONF_REQ_PRIM = {
    .name = "LME_CONF_REQ",
    .prim_seq = LME_CONF_REQ,
    .SAP = {L_SAPC, NULL, NULL},
    .req_cb = {L_SAPC_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t LME_AUTH_REQ_PRIM = {
    .name = "LME_AUTH_REQ",
    .prim_seq = LME_AUTH_REQ,
    .SAP = {L_SAPC, NULL, NULL},
    .req_cb = {L_SAPC_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t LME_STATE_IND_PRIM = {
    .name = "LME_STATE_IND",
    .prim_seq = LME_STATE_IND,
    .SAP = {NULL, NULL, NULL},
    .req_cb = {L_SAPC_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t LME_TEST_REQ_PRIM = {
    .name = "LME_TEST_REQ",
    .prim_seq = LME_TEST_REQ,
    .SAP = {L_SAPT, NULL, NULL},
    .req_cb = {L_SAPT_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t LME_R_REQ_PRIM = {
    .name = "LME_R_REQ",
    .prim_seq = LME_R_REQ,
    .SAP = {L_SAPR, NULL, NULL},
    .req_cb = {L_SAPR_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t LME_R_IND_PRIM = {
    .name = "LME_R_IND",
    .prim_seq = LME_R_IND,
    .SAP = {NULL, NULL, NULL},
    .req_cb = {L_SAPR_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

/**
 * TODO: 测试阶段，直接赋值。未来需要和GSC进行交互
 * @return
 */
static uint32_t get_gs_sac() {
    return 0x0ABD; /* 12bits */
}

static uint32_t get_as_ua() {
    return generate_urand(28); /* 28 bits */
}


l_err make_lme_layer() {
    switch (config.role) {
        case LD_AS:
        case LD_GS: {
            switch (config.role) {
                case LD_AS: {
                    /* AS set the initial state 'FSCANNING' */
                    init_lme_fsm(&lme_layer_objs, LME_FSCANNING);
                    lme_layer_objs.lme_as_man = init_as_man(DEFAULT_SAC, config.UA, DEFAULT_SAC);
                    init_as_snf_layer(as_finish_auth_func, trans_snp_data, register_snf_failed);
                    break;
                }
                case LD_GS: {
                    config.is_merged == TRUE
                        ? init_gs_snf_layer(get_gs_sac(), config.gsnf_addr_v6, config.gsnf_remote_port,
                                            config.gsnf_local_port, trans_snp_data, NULL)
                        : init_gs_snf_layer_unmerged(get_gs_sac(), config.gsnf_addr_v6, config.gsnf_remote_port,
                                                     config.gsnf_local_port,
                                                     trans_snp_data, register_snf_failed);

                    /* GS set the initial state 'OPEN' */
                    init_lme_fsm(&lme_layer_objs, LME_OPEN);
                    lme_layer_objs.GS_SAC = get_gs_sac();
                    lme_layer_objs.LME_GS_AUTH_AS = init_lme_sac_map();

                    break;
                }
                default:
                    break;
            }
            init_lme_mms(&lme_layer_objs);
            init_lme_rms(&lme_layer_objs);
            break;
        }
        case LD_SGW: {
            /* 默认AS的UA为10010，SGW的UA为10000 */
            UA_STR(ua_as);
            UA_STR(ua_sgw);
            //            embed_rootkey(LD_SGW, get_ua_str(10010, ua_as), get_ua_str(config.UA, ua_sgw));

            init_sgw_snf_layer(config.port);
        }
        default:
            break;
    }


    return LD_OK;
}

void L_SAPC(ld_prim_t *prim) {
    switch (prim->prim_seq) {
        case LME_OPEN_REQ: {
            if (prim->prim_obj_typ == RC_TYP_OPEN) {
                if (config.role == LD_AS) {
                    /* call MAC layer to fscan */
                    fscanning_freqs_t fscan_freqs;
                    /* activate the lower layer execute FSCAN process */
                    if ((prim->prim_err = preempt_prim(&MAC_FSCAN_REQ_PRIM, E_TYP_ANY, &fscan_freqs, NULL, 0, 0))) {
                        log_error("LME can not call lower layers manipulate FSCANNING");
                        return;
                    }

                    /* if success, change LME state to CSCANNING */
                    if ((prim->prim_err = change_state(&lme_layer_objs.lme_fsm, LME_EV_DEFAULT,
                                                       &(fsm_event_data_t){&lme_fsm_events[LME_CSCANNING], NULL}))) {
                        log_error("LME can not change state from LME_FSCANNING to LME_CSACNNING correctly");
                        return;
                    }

                    /* 等待接收到CONNECTING完成信号 */
                    while (lme_layer_objs.finish_status != LME_CONNECTING_FINISHED) usleep(100000);
                } else {
                    if (!in_state(&lme_layer_objs.lme_fsm, lme_fsm_states[LME_OPEN])) {
                        prim->prim_err = LD_ERR_WRONG_STATE;
                        return;
                    }

                    start_mms();
                }
                break;
            } else if (prim->prim_obj_typ == RC_TYP_CLOSE) {
                if (!in_state(&lme_layer_objs.lme_fsm, lme_fsm_states[LME_OPEN]) && !in_state(
                        &lme_layer_objs.lme_fsm, lme_fsm_states[LME_AUTH])) {
                    prim->prim_err = LD_ERR_WRONG_STATE;
                    break;
                }
                /* if success, change LME state to CSCANNING */
                if ((prim->prim_err = change_state(&lme_layer_objs.lme_fsm, LME_EV_DEFAULT,
                                                   &(fsm_event_data_t){&lme_fsm_events[LME_FSCANNING], NULL}))) {
                    log_error("LME can not change state from LME_FSCANNING to LME_CSACNNING correctly");
                    return;
                }
                break;
            } else {
                log_warn("Wrong LME_OPEN_REQ type, Please Check!");
                break;
            }
        }
        case LME_AUTH_REQ: {
            if ((prim->prim_err = change_state(&lme_layer_objs.lme_fsm, LME_EV_DEFAULT,
                                               &(fsm_event_data_t){&lme_fsm_events[LME_AUTH], NULL}))) {
                log_error("LME can not change state from LME_CONNECTING to LME_AUTH correctly");
                return;
            }

            /* 等待接收到AUTH完成信号 */
            while (lme_layer_objs.finish_status != LME_AUTH_FINISHED) usleep(100000);


            break;
        }
        default:
            break;
    }
}

void M_SAPI_cb(ld_prim_t *prim) {
    switch (prim->prim_seq) {
        case MAC_FSCAN_REQ: {
            fscanning_freqs_t *fscan_freqs = prim->prim_objs;
            if (prim->prim_err) return;
            if (!in_state(&lme_layer_objs.lme_fsm, lme_fsm_states[LME_FSCANNING])) {
                //TODO: 错误处理
                log_error("Wrong State!");
                prim->prim_err = LD_ERR_WRONG_STATE;
                return;
            }
            /* 设置初始频率，未来需要改进 */
            lme_layer_objs.init_flf = fscan_freqs->avial_fl_freqs[0];
            lme_layer_objs.init_rlf = fscan_freqs->avial_rl_freqs[0];

            break;
        }
        default: {
            break;
        }
    }
}

void L_SAPT(ld_prim_t *prim) {
}


l_err entry_LME_CSCANNING(void *args) {
    l_err err = LD_OK;

    do {
        /* activate the lower layer execute CSCAN process */
        if ((err = preempt_prim(&MAC_CSCAN_REQ_PRIM, E_TYP_ANY, NULL, NULL, 0, 0))) {
            log_error("LME can not call lower layers manipulate FSCANNING");
            break;
        }

        if ((err = preempt_prim(&LME_STATE_IND_PRIM, LME_STATE_CHANGE, &(lme_state_chg_t){
                                    .ua = lme_layer_objs.lme_as_man->AS_UA,
                                    .sac = lme_layer_objs.lme_as_man->AS_SAC,
                                    .state = LME_CSCANNING,
                                }, NULL, 0, 0))) {
            log_error("LME can not call RCU current state");
            break;
        }
    } while (0);
    return err;
}


l_err entry_LME_CONNECTING(void *args) {
    /* Tell RCU the state of LME is CONNECTING */
    l_err err = LD_OK;

    do {
        if ((err = preempt_prim(&MAC_CONNECT_REQ_PRIM, E_TYP_ANY, NULL, NULL, 0, 0))) {
            log_error("LME can not call lower layers manipulate FSCANNING");
            break;
        }
        if ((err = preempt_prim(&LME_STATE_IND_PRIM, LME_STATE_CHANGE, &(lme_state_chg_t){
                                    .ua = lme_layer_objs.lme_as_man->AS_UA,
                                    .sac = lme_layer_objs.lme_as_man->AS_SAC,
                                    .state = LME_CONNECTING
                                }, NULL, 0, 0))) {
            log_error("LME can not call RCU current state");
            break;
        }
        start_mms();
    } while (0);
    return err;
}


l_err change_LME_CONNECTING() {
    // /* 等待接收到CSCANNING完成信号 */

    l_err err = LD_OK;
    if ((err = change_state(&lme_layer_objs.lme_fsm, LME_EV_DEFAULT,
                            &(fsm_event_data_t){&lme_fsm_events[LME_CONNECTING], NULL}))) {
        log_error("LME can not change state from LME_CSCANNING to LME_CONNECTING correctly");
    }
    return err;
}

int8_t as_finish_auth_func() {
    change_LME_OPEN();
    lme_layer_objs.finish_status = LME_AUTH_FINISHED;
    return 0;
}

l_err change_LME_OPEN() {
    l_err err = LD_OK;

    if ((err = change_state(&lme_layer_objs.lme_fsm, LME_EV_DEFAULT,
                            &(fsm_event_data_t){&lme_fsm_events[LME_OPEN], NULL}))) {
        log_error("LME can not change state from LME_AUTH to LME_OPEN correctly");
    }

    return err;
}

void exit_LME_CONN_OPEN_action(void *curr_st_data, struct sm_event_s *event, void *new_state_data) {
    if (config.role != LD_AS) return;
    dc_cell_exit_t exit = {
        .d_type = DC_TYP_CELL_EXIT,
        .SAC = lme_layer_objs.lme_as_man->AS_SAC,
    };
    preempt_prim(&MAC_DCCH_REQ_PRIM, DC_TYP_CELL_EXIT,
                 gen_pdu(&exit, dc_format_descs[DC_TYP_CELL_EXIT].f_desc, "dc cell exit"), NULL, 0, 0);
}

void SN_SAPC_cb(ld_prim_t *prim) {
}

void SN_SAPD_L_cb(ld_prim_t *prim) {
    switch (prim->prim_seq) {
        case SN_DATA_IND: {
            orient_sdu_t *osdu = prim->prim_objs;
            upload_snf(prim->prim_obj_typ == VER_PASS, osdu->AS_SAC, osdu->GS_SAC, osdu->buf->ptr, osdu->buf->len);
            break;
        }
        default:
            break;
    }
}

l_err entry_LME_AUTH(void *args) {
    l_err err;

    do {
        //change MAC state into MAC_AUTH
        if ((err = preempt_prim(&MAC_AUTH_REQ_PRIM, E_TYP_ANY, NULL, NULL, 0, 0))) {
            log_error("LME can not call mac layers manipulate AUTH");
            break;
        }

        //change SNP state into SNP_AUTH
        if ((err = preempt_prim(&SN_AUTH_REQ_PRIM, E_TYP_ANY, NULL, NULL, 0, 0))) {
            log_error("LME can not call snp layers manipulate AUTH");
            break;
        }

        /* Tell RCU the state of LME is AUTH */
        if ((err = preempt_prim(&LME_STATE_IND_PRIM, LME_STATE_CHANGE,
                                &(lme_state_chg_t){
                                    .ua = lme_layer_objs.lme_as_man->AS_UA,
                                    .sac = lme_layer_objs.lme_as_man->AS_SAC,
                                    .state = LME_AUTH
                                }, NULL, 0, 0))) {
            log_error("LME can not call RCU current state");
            break;
        }

        if (snf_LME_AUTH(
                config.role,
                lme_layer_objs.lme_as_man->AS_SAC,
                lme_layer_objs.lme_as_man->AS_UA,
                lme_layer_objs.lme_as_man->AS_CURR_GS_SAC
            ) != LDCAUC_OK) {
            err = LD_ERR_INTERNAL;
        }
    } while (0);
    return err;
}


l_err entry_LME_OPEN(void *args) {
    l_err err = LD_OK;
    do {
        /* change SNP state into SNP_OPEN */
        if ((err = preempt_prim(&SN_OPEN_REQ_PRIM, E_TYP_ANY, NULL, NULL, 0, 0))) {
            log_error("LME can not call snp layers manipulate OPEN");
            break;
        }

        /* Tell RCU the state of LME is OPEN */
        if ((err = preempt_prim(&LME_STATE_IND_PRIM, LME_STATE_CHANGE,
                                &(lme_state_chg_t){
                                    .ua = lme_layer_objs.lme_as_man->AS_UA,
                                    .sac = lme_layer_objs.lme_as_man->AS_SAC,
                                    .state = LME_OPEN
                                }, NULL, 0, 0))) {
            log_error("LME can not call RCU current state");
            break;
        }
    } while (0);
    return err;
}

int8_t trans_snp_data(uint16_t AS_SAC, uint16_t GS_SAC, uint8_t *buf, size_t buf_len) {
    orient_sdu_t *orient_sdu = create_orient_sdus(AS_SAC, GS_SAC);

    /* 通过原语向SNP层传递对应报文 */
    CLONE_TO_CHUNK(*orient_sdu->buf, buf, buf_len);
    preempt_prim(&SN_DATA_REQ_PRIM, SN_TYP_FROM_LME, orient_sdu, free_orient_sdus, 0, 0);
    return LD_OK;
}

int8_t register_snf_failed(uint16_t AS_SAC) {
    if (config.role == LD_AS) {
    } else {
        delete_lme_as_node_by_sac(AS_SAC, clear_as_man);
    }
    return LD_OK;
}
