//
// Created by 邹嘉旭 on 2024/2/21.
//
#include "ldacs_phy.h"

static l_err start_fast_scanning(fscanning_freqs_t *freqs);

static void init_merge_buffers();

static phy_sim_t json_sim = {
    .sim_level = PHY_SIM_JSON,
    .init_sim = init_sim_json,
    .upward_process = upward_json_sim,
    .downward_process = downward_json_sim,
};

static phy_sim_t real_sim = {
    .sim_level = PHY_SIM_REAL,
    .init_sim = init_sim_real,
    .upward_process = upward_real,
    .downward_process = downward_real,
};

void as_init_gtimer() {
    if (gtimer.is_reg == FALSE) {
        register_gtimer(&gtimer);
        register_gtimer_event(&gtimer, &phy_layer_objs.gtimer[PHY_TIMER_RA]);
    } else {
        reregister_gtimer(&gtimer);
    }
}

phy_layer_objs_t phy_layer_objs = {
    .PHY_N_FFT = 64,
    .PHY_T_SA = 1600,
    .PHY_DEL_F = 9.765625,
    .need_sync = TRUE,

    .phy_g_timer = {{.it_interval = {0, MF_TIMER}, .it_value = {0, 0}}},
    .gtimer = {
        {trans_bc_timer_func, NULL,TIMER_INFINITE},
        {trans_ra_timer_func, NULL,TIMER_INFINITE},
        {trans_cc_timer_func, NULL, 4},
        {trans_dc_timer_func, NULL, 4},
    },

    .cc_dch_merge = {
        .has_cc = FALSE,
        .has_data = FALSE,
        .mutex = PTHREAD_MUTEX_INITIALIZER,
    },

    .device_args = {
        .dev_en = &phy_layer_objs.dev,
        .process_pkt = process_phy_pkt,
    },
};

static void free_malloced_ptr(void *ptr) {
    if (ptr) {
        free(ptr);
    }
}


l_err make_phy_layer(enum PHY_SIM_LEVEL level) {
    init_merge_buffers();

    switch (level) {
        case PHY_SIM_JSON:
            phy_layer_objs.sim = &json_sim;
            break;
        case PHY_SIM_REAL:
            phy_layer_objs.sim = &real_sim;
            break;
        default:
            return LD_ERR_WRONG_PARA;
    }

    switch (config.role) {
        case LD_AS:
            phy_layer_objs.sim->init_sim(&phy_layer_objs);
            break;
        case LD_GS: {
            register_gtimer(&gtimer);
            register_gtimer_event(&gtimer, &phy_layer_objs.gtimer[0]);
            phy_layer_objs.sim->init_sim(&phy_layer_objs);

            /* GS start recieve directly */
            if (set_device("UDP", &phy_layer_objs.dev)) {
                exit(0);
            }
            pthread_create(&phy_layer_objs.recv_th, NULL, start_recv, &phy_layer_objs.device_args);
            pthread_detach(phy_layer_objs.recv_th);
            break;
        }
        default:
            break;
    }

    return LD_OK;
}

static void *trans_bc_timer_func(void *args) {
    register_gtimer(&phy_layer_objs.phy_g_timer);
    register_gtimer_event(&phy_layer_objs.phy_g_timer, &phy_layer_objs.gtimer[2]);

    p_rtx_obj_t *rtx_bc_obj = malloc(sizeof(p_rtx_obj_t));
    rtx_bc_obj->rtx_type = PHY_RTX_BC_IND;
    preempt_prim(&PHY_RTX_BC_IND_PRIM, E_TYP_ANY, rtx_bc_obj, free_malloced_ptr, 0, 0);

    return NULL;
}

static void *trans_cc_timer_func(void *args) {
    p_rtx_obj_t *rtx_cc_obj = malloc(sizeof(p_rtx_obj_t));
    rtx_cc_obj->rtx_type = PHY_RTX_CC_IND;
    usleep(5000);
    if (preempt_prim(&PHY_RTX_CC_IND_PRIM, E_TYP_ANY, rtx_cc_obj, free_malloced_ptr, 0, 0) != LD_OK) {
        return NULL;
    }

    trans_fl_data_timer_func(NULL);

    return NULL;
}

static void *trans_ra_timer_func(void *args) {
    register_gtimer(&phy_layer_objs.phy_g_timer);
    register_gtimer_event(&phy_layer_objs.phy_g_timer, &phy_layer_objs.gtimer[3]);

    p_rtx_obj_t *rtx_ra_obj = malloc(sizeof(p_rtx_obj_t));
    rtx_ra_obj->rtx_type = PHY_RTX_RA_IND;
    preempt_prim(&PHY_RTX_RA_IND_PRIM, E_TYP_ANY, rtx_ra_obj, free_malloced_ptr, 0, 0);
    return NULL;
}

static void *trans_dc_timer_func(void *args) {
    /*等一会CC*/
    // log_warn("============== DC / RL_DCH Indication ==============");
    usleep(10000);

    p_rtx_obj_t *rtx_dc_obj = malloc(sizeof(p_rtx_obj_t));
    rtx_dc_obj->rtx_type = PHY_RTX_DC_IND;
    preempt_prim(&PHY_RTX_DC_IND_PRIM, E_TYP_ANY, rtx_dc_obj, free_malloced_ptr, 0, 0);

    /* start a rl data slot after dc */
    trans_rl_data_timer_func(NULL);
    return NULL;
}

static void trans_fl_data_timer_func(void *args) {
    // log_debug("=== FL IND");
    p_rtx_obj_t *rtx_data_obj = malloc(sizeof(p_rtx_obj_t));
    rtx_data_obj->rtx_type = PHY_RTX_DATA_IND;
    preempt_prim(&PHY_RTX_DATA_IND_PRIM, E_TYP_ANY, rtx_data_obj, free_malloced_ptr, 0, 0);
}

static void trans_rl_data_timer_func(void *args) {
    p_rtx_obj_t *rtx_data_obj = malloc(sizeof(p_rtx_obj_t));
    rtx_data_obj->rtx_type = PHY_RTX_DATA_IND;
    preempt_prim(&PHY_RTX_DATA_IND_PRIM, E_TYP_ANY, rtx_data_obj, free_malloced_ptr, 0, 0);
}


static void init_merge_buffers() {
    cc_dch_merge_t *merge = &phy_layer_objs.cc_dch_merge;

    uint8_t *zeros = calloc(FL_DATA_BLK_2_F_LEN_MAX, sizeof(uint8_t));

    INIT_BUF_ARRAY_UNPTR(merge->cc_buf, CC_BLK_N);
    INIT_BUF_ARRAY_UNPTR(merge->data_bufs, FL_DATA_BLK_N);

    INIT_BUF_ARRAY_UNPTR(merge->data_next_mf, 2);
    CLONE_TO_CHUNK(*merge->data_next_mf[0], zeros, FL_DATA_BLK_2_F_LEN_MAX);
    CLONE_TO_CHUNK(*merge->data_next_mf[1], zeros, FL_DATA_BLK_2_F_LEN_MAX);
    //INIT_BUF_ARRAY_PTR(merge->data_next_mf, 2, FL_DATA_BLK_2_F_LEN_MAX);
    free(zeros);
}

static void clear_merge_buffers() {
    cc_dch_merge_t *merge = &phy_layer_objs.cc_dch_merge;
    FREE_BUF_ARRAY(merge->cc_buf, CC_BLK_N);
    FREE_BUF_ARRAY(merge->data_bufs, FL_DATA_BLK_N);
}

/**
 * combine the CCCH and DCH in one MF
 * @param seq
 * @param to_trans
 * @return
 */
static buffer_t **combine_cc_data(uint16_t seq, buffer_t **to_trans) {
    cc_dch_merge_t *merge = &phy_layer_objs.cc_dch_merge;
    buffer_t **out_buf = NULL;
    ld_lock(&merge->mutex);

    /* when both CCCH and DCH have data, then goto merge section. (Scarcely Possible) */
    if (merge->has_cc && merge->has_data) goto merge;

    switch (seq) {
        case PHY_CC_REQ:
            /* set the CCCH data */
            merge->has_cc = TRUE;
            CLONE_BY_BUFFER(*merge->cc_buf[0], *to_trans[0]);
            break;
        case PHY_DATA_REQ:
            /* set the DCH data */
            merge->has_data = TRUE;
            CLONE_BY_BUFFER(*merge->data_bufs[0], *to_trans[0]);
            CLONE_BY_BUFFER(*merge->data_bufs[1], *to_trans[1]);
            CLONE_BY_BUFFER(*merge->data_bufs[2], *to_trans[2]);
            CLONE_BY_BUFFER(*merge->data_bufs[3], *to_trans[3]);
            break;
        default:
            /* wrong params */
            goto finish;
    }
    /* when only one channel data is ready, keep waiting */
    if (!merge->has_cc || !merge->has_data) {
        // log_warn("!!!!!");
        goto finish;
    }
merge:
    /* while has_cc is TRUE and has_data is TRUE */
    {
        out_buf = malloc(FL_DATA_BLK_N * sizeof(buffer_t *));
        INIT_BUF_ARRAY_UNPTR(out_buf, FL_DATA_BLK_N);

        /* Clone the previous MF data in slot 1-12*/
        CLONE_BY_BUFFER(*out_buf[0], *merge->data_next_mf[0]);
        CLONE_BY_BUFFER(*out_buf[1], *merge->data_next_mf[1]);
        /* Clone the CC data in slot 13-21, then cat the DCH data after it */
        CLONE_BY_BUFFER(*out_buf[2], *merge->cc_buf[0]);
        cat_to_buffer(out_buf[2], merge->data_bufs[0]->ptr, merge->data_bufs[0]->total);

        /* Clone the DCH data in slot 22-27*/
        CLONE_BY_BUFFER(*out_buf[3], *merge->data_bufs[1]);

        /* clear previous MF data and set the new DCH data */
        buffer_clear(merge->data_next_mf[0]);
        buffer_clear(merge->data_next_mf[1]);
        CLONE_BY_BUFFER(*merge->data_next_mf[0], *merge->data_bufs[2]);
        CLONE_BY_BUFFER(*merge->data_next_mf[1], *merge->data_bufs[3]);


        /* clear CC data and DCH data */
        clear_merge_buffers();
        merge->has_cc = FALSE;
        merge->has_data = FALSE;
    }
finish:
    ld_unlock(&merge->mutex);
    return out_buf;
}

ld_prim_t PHY_RA_REQ_PRIM = {
    .name = "PHY_RA_REQ",
    .prim_seq = PHY_RA_REQ,
    .SAP = {P_SAPD, NULL, NULL},
    .req_cb = {P_SAPD_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t PHY_RA_IND_PRIM = {
    .name = "PHY_RA_IND",
    .prim_seq = PHY_RA_IND,
    .SAP = {NULL, NULL, NULL},
    .req_cb = {P_SAPD_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t PHY_RTX_RA_IND_PRIM = {
    .name = "PHY_RTX_RA_IND",
    .prim_seq = PHY_RTX_RA_IND,
    .SAP = {NULL, NULL, NULL},
    .req_cb = {P_SAPT_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t PHY_DC_REQ_PRIM = {
    .name = "PHY_DC_REQ",
    .prim_seq = PHY_DC_REQ,
    .SAP = {P_SAPD, NULL, NULL},
    .req_cb = {P_SAPD_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t PHY_DC_IND_PRIM = {
    .name = "PHY_DC_IND",
    .prim_seq = PHY_DC_IND,
    .SAP = {NULL, NULL, NULL},
    .req_cb = {P_SAPD_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};


ld_prim_t PHY_RTX_DC_IND_PRIM = {
    .name = "PHY_RTX_DC_IND",
    .prim_seq = PHY_RTX_DC_IND,
    .SAP = {NULL, NULL, NULL},
    .req_cb = {P_SAPT_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};


ld_prim_t PHY_CC_REQ_PRIM = {
    .name = "PHY_CC_REQ",
    .prim_seq = PHY_CC_REQ,
    .SAP = {P_SAPD, NULL, NULL},
    .req_cb = {P_SAPD_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t PHY_CC_IND_PRIM = {
    .name = "PHY_CC_IND",
    .prim_seq = PHY_CC_IND,
    .SAP = {NULL, NULL, NULL},
    .req_cb = {P_SAPD_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t PHY_RTX_CC_IND_PRIM = {
    .name = "PHY_RTX_CC_IND",
    .prim_seq = PHY_RTX_CC_IND,
    .SAP = {NULL, NULL, NULL},
    .req_cb = {P_SAPT_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t PHY_BC_REQ_PRIM = {
    .name = "PHY_BC_REQ",
    .prim_seq = PHY_BC_REQ,
    .SAP = {P_SAPD, NULL, NULL},
    .req_cb = {P_SAPD_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};


ld_prim_t PHY_BC_IND_PRIM = {
    .name = "PHY_BC_IND",
    .prim_seq = PHY_BC_IND,
    .SAP = {NULL, NULL, NULL},
    .req_cb = {P_SAPD_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t PHY_RTX_BC_IND_PRIM = {
    .name = "PHY_RTX_BC_IND",
    .prim_seq = PHY_RTX_BC_IND,
    .SAP = {NULL, NULL, NULL},
    .req_cb = {P_SAPT_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};


ld_prim_t PHY_DATA_REQ_PRIM = {
    .name = "PHY_DATA_REQ",
    .prim_seq = PHY_DATA_REQ,
    .SAP = {P_SAPD, NULL, NULL},
    .req_cb = {P_SAPD_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t PHY_DATA_IND_PRIM = {
    .name = "PHY_DATA_IND",
    .prim_seq = PHY_DATA_IND,
    .SAP = {NULL, NULL, NULL},
    .req_cb = {P_SAPD_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};


ld_prim_t PHY_RTX_DATA_IND_PRIM = {
    .name = "PHY_RTX_DATA_IND",
    .prim_seq = PHY_RTX_DATA_IND,
    .SAP = {NULL, NULL, NULL},
    .req_cb = {P_SAPT_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};


ld_prim_t PHY_FSCAN_REQ_PRIM = {
    .name = "PHY_FSCAN_REQ",
    .prim_seq = PHY_FSCAN_REQ,
    .SAP = {P_SAPC, NULL, NULL},
    .req_cb = {P_SAPC_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};


ld_prim_t PHY_CSCAN_REQ_PRIM = {
    .name = "PHY_CSAN_REQ",
    .prim_seq = PHY_CSCAN_REQ,
    .SAP = {P_SAPC, NULL, NULL},
    .req_cb = {P_SAPC_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};


ld_prim_t PHY_GSCAN_REQ_PRIM = {
    .name = "PHY_GSCAN_REQ",
    .prim_seq = PHY_GSCAN_REQ,
    .SAP = {P_SAPC, NULL, NULL},
    .req_cb = {NULL, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};


ld_prim_t PHY_RDY_TO_SCAN_IND_PRIM = {
    .name = "PHY_RDY_TO_SCAN_IND",
    .prim_seq = PHY_RDY_TO_SCAN_IND,
    .SAP = {P_SAPC, NULL, NULL},
    .req_cb = {NULL, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t PHY_CONF_REQ_PRIM = {
    .name = "PHY_CONF_REQ",
    .prim_seq = PHY_CONF_REQ,
    .SAP = {P_SAPC, NULL, NULL},
    .req_cb = {NULL, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};


ld_prim_t PHY_CONF_IND_PRIM = {
    .name = "PHY_CONF_IND",
    .prim_seq = PHY_CONF_IND,
    .SAP = {P_SAPC, NULL, NULL},
    .req_cb = {NULL, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t PHY_FLSYNC_IND_PRIM = {
    .name = "PHY_FLSYNC_IND",
    .prim_seq = PHY_FLSYNC_IND,
    .SAP = {NULL, NULL, NULL},
    .req_cb = {P_SAPS_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t PHY_SYNC_REQ_PRIM = {
    .name = "PHY_SYNC_REQ",
    .prim_seq = PHY_SYNC_REQ,
    .SAP = {P_SAPS, NULL, NULL},
    .req_cb = {P_SAPS_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t PHY_SYNC_IND_PRIM = {
    .name = "PHY_SYNC_IND",
    .prim_seq = PHY_SYNC_IND,
    .SAP = {NULL, NULL, NULL},
    .req_cb = {P_SAPS_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};


void P_SAPC(ld_prim_t *prim) {
    switch (prim->prim_seq) {
        case PHY_FSCAN_REQ: {
            /* scanning the aviable freqency */
            prim->prim_err = start_fast_scanning(prim->prim_objs);
            break;
        }
        case PHY_CSCAN_REQ: {
            /* setting device and start getting BC from FL */
            if (set_device("UDP", &phy_layer_objs.dev)) {
                exit(0);
            }
            if (pthread_create(&phy_layer_objs.recv_th, NULL, start_recv, &phy_layer_objs.device_args) != 0) {
                prim->prim_err = LD_ERR_THREAD;
            }
            pthread_detach(phy_layer_objs.recv_th);
            break;
        }
        case PHY_GSCAN_REQ: {
            break;
        }
        case PHY_CONF_REQ: {
            if (prim->prim_obj_typ == PHY_TYP_HO) {
                //默认为1111.0和965.0,未来频率会被指定在Obj中
                if (!set_new_freq(&phy_layer_objs.dev, 1111.0, FL)) {
                    log_error("Cannot set new Freqency for FL.");
                    prim->prim_err = LD_ERR_INTERNAL;
                }
                if (!set_new_freq(&phy_layer_objs.dev, 965.0, RL)) {
                    log_error("Cannot set new Freqency for RL.");
                    prim->prim_err = LD_ERR_INTERNAL;
                }
            }
            break;
        }
        default:
            break;
    }
}

void P_SAPD(ld_prim_t *prim) {
    sdu_s_t *sdus = prim->prim_objs;
    buffer_t **in_bufs = NULL;
    buffer_t *out_buf = NULL;
    bool is_combind_cc = FALSE;

    switch (prim->prim_seq) {
        case PHY_BC_REQ:
        case PHY_RA_REQ:
        case PHY_DC_REQ:
            in_bufs = sdus->blks;
            break;
        case PHY_CC_REQ:
        case PHY_DATA_REQ:
            //内存泄漏
            if (prim->prim_obj_typ == E_TYP_RL) {
                in_bufs = sdus->blks;
            } else {
                is_combind_cc = TRUE;
                if ((in_bufs = combine_cc_data(prim->prim_seq, sdus->blks)) == NULL) goto end;
            }
            break;
        default:
            return;
    }

    if ((prim->prim_err = phy_layer_objs.sim->downward_process(prim, in_bufs, &out_buf)) == LD_OK) {
        if (out_buf) {
            prim->prim_err = phy_layer_objs.dev.send_pkt(out_buf, (config.role == LD_AS) ? RL : FL);
        }
    }

end:
    if (out_buf) {
        free_buffer(out_buf);
    }

    /* only for CC-FL */
    if (is_combind_cc) {
        if (in_bufs) {
            FREE_BUF_ARRAY_DEEP2(in_bufs, FL_DATA_BLK_N);
        }
    }
}

void P_SAPT(ld_prim_t *prim) {
}

void P_SAPS(ld_prim_t *prim) {
    buffer_t *out_buf = NULL;
    if ((prim->prim_err = phy_layer_objs.sim->downward_process(prim, NULL, &out_buf)) == LD_OK) {
        if (out_buf) {
            prim->prim_err = phy_layer_objs.dev.send_pkt(out_buf, (config.role == LD_AS) ? RL : FL);
        }
    }

    if (out_buf) {
        free_buffer(out_buf);
    }
}

void process_phy_pkt(void *data) {
    phy_layer_objs.sim->upward_process(data);
}

static l_err start_fast_scanning(fscanning_freqs_t *freqs) {
    //模拟扫描时间
    usleep(500000);
    freqs->avial_fl_freqs[0] = config.init_fl_freq;
    freqs->avial_rl_freqs[0] = config.init_rl_freq;

    return LD_OK;
}
