//
// Created by 邹嘉旭 on 2024/1/14.
//


#include "ldacs_lme.h"

enum lme_rms_timer_index {
    LME_RMS_TIMER_RRM = 0,
};


void *rrm_func(void *args);

static void trans_cc_func(void *args);

static void trans_cc_must_timer_func(evutil_socket_t fd, short event, void *arg);

static void resource_rl_alloc_cb(ld_drr_t *drr, size_t *map, void *args);

static void resource_fl_alloc_cb(ld_drr_t *drr, size_t *map, void *args);

typedef struct lme_rms_obj_s {
    lme_layer_objs_t *lme_obj;

    // lfqueue_t *req_queue;
    lfqueue_t *cc_queue;
    pthread_t rrm_th;

    ld_bitset_t *CO_bitset;
    uint8_t CO_successor;

    ld_drr_t *rl_drr,
            *fl_drr;

    size_t cc_sz;
    size_t BO;

    stimer_ev_t stimer[10];
} lme_rms_obj_t;


static lme_rms_obj_t lme_rms_obj = {
        .lme_obj = NULL,
        .CO_successor = 0,

        .stimer = {
                {trans_cc_must_timer_func, NULL, CC_MAC_INTVL},
        },
};


static l_err init_co_res(ld_bitset_t *bitset) {
    uint16_t *res = malloc(sizeof(uint16_t) * bitset->res_num);
    for (int i = 0; i < bitset->res_num; i++) {
        res[i] = i;
    }
    bitset->resources = res;
    return LD_OK;
}

static void free_co_res(void *res) {
    free(res);
}

static uint16_t get_CO() {
    uint16_t *co = NULL;
    bs_alloc_resource(lme_rms_obj.CO_bitset, (void **) &co);
    return *co;
}

void free_CO(uint16_t co) {
    bs_free_resource(lme_rms_obj.CO_bitset, co);
}


l_err init_lme_rms(lme_layer_objs_t *obj) {
    lme_rms_obj.lme_obj = obj;
    // lme_rms_obj.req_queue = lfqueue_init();
    lme_rms_obj.cc_queue = lfqueue_init();

    switch (config.role) {
        case LD_AS:
            break;
        case LD_GS: {
            lme_rms_obj.rl_drr = init_ld_drr(SAC_MAX);
            lme_rms_obj.fl_drr = init_ld_drr(SAC_MAX);
            lme_rms_obj.CO_bitset = init_bitset(CO_MAX, sizeof(uint16_t), init_co_res, free_co_res);
            break;
        }
        default:
            break;
    }

    return LD_OK;
}

static void trans_cc_must_timer_func(evutil_socket_t fd, short event, void *arg) {
    trans_cc_sd_timer_func(NULL);
    trans_cc_mac_timer_func(NULL);
    // if (lme_rms_obj.BO != 0)
    //     log_warn("!!!!!!!!!!! %d", lme_rms_obj.BO);
}

static void trans_cc_func(void *args) {
    lme_rms_obj.cc_sz = 0;
    lme_rms_obj.BO = 0;

    /* start timer for cc mac. */

    register_stimer(&lme_rms_obj.stimer[0]);

    // trans_cc_sd_timer_func(NULL);

    /* 10.6.4.2 Note that the DCCH descriptor message is not required if no aircraft are registered in the cell */
    if (bs_get_alloced(lme_rms_obj.CO_bitset) != 0) {
        cc_dcch_desc_t cc_dd;
        zero(&cc_dd);
        trans_cc_dd_func(&cc_dd);

        /* 首先分配非混合的18个dc slot，剩下的混合槽考虑在cc_must_timer_func里最后分配 */
        drr_resource_alloc(lme_rms_obj.fl_drr, 200, FL_DATA_BLK_2_F_LEN_MAX * 3, 0, resource_fl_alloc_cb,
                           &lme_rms_obj.BO);

        size_t rl_avail_blks = RL_SLOT_MAX - cc_dd.COL;
        ld_rpso_t *mac_rpso_sac = init_rpsos(cc_dd.COL, rl_avail_blks);
        /* 在这里做资源分配drr */
        drr_resource_alloc(lme_rms_obj.rl_drr, 200, rl_avail_blks * RL_DATA_BLK_LEN_MAX, 0, resource_rl_alloc_cb,
                           mac_rpso_sac);
        set_rpsos(mac_rpso_sac);
    }
}

void *trans_lme_cc_timer_func(void *args) {
    trans_cc_func(NULL);
    return NULL;
}


uint8_t cc_count = 0;

void trans_cc_sd_timer_func(void *args) {
    uint16_t highest_co = bs_get_highest(lme_rms_obj.CO_bitset);

    /* add slot_desc length and the hmac length */
    lme_rms_obj.cc_sz += cc_format_descs[C_TYP_SLOT_DESC].desc_size + (get_sec_maclen(SEC_MACLEN_64) + 1);

    /* TODO:  CCL更改 */
    cc_slot_desc_t sd_n = {
            .CCL = ((lme_rms_obj.cc_sz - 1) / CC_BLK_LEN_MIN) + 1,
            .DCL = highest_co + 1 >= DCL_MAX ? DCL_MAX : highest_co + 1
    };

    //TODO: 这里和MAC的对应位置改成和DCCH DESC一样的
    // preempt_prim(&MAC_CCCH_REQ_PRIM, C_TYP_SLOT_DESC,
    //              gen_pdu(&sd_n, cc_format_descs[C_TYP_SLOT_DESC].f_desc, "CCSD OUT"), NULL, 0, 0);
    preempt_prim(&MAC_CCCH_REQ_PRIM, C_TYP_SLOT_DESC, &sd_n, NULL, 0, 0);
}

void trans_cc_dd_func(void *args) {
    uint16_t highest_co = bs_get_highest(lme_rms_obj.CO_bitset);
    cc_dcch_desc_t *dd_n = args;

    dd_n->c_type = C_TYP_DCCH_DESC;
    dd_n->COL = highest_co + 1 >= DCL_MAX ? DCL_MAX : highest_co + 1;
    /* 10.6.4.2   COS should be set to the successor of the last CO served in the previous MF. */
    dd_n->COS = lme_rms_obj.CO_successor;
    /* 10.6.4.2   COM must be equal to the highest assigned CO+1. */
    dd_n->COM = highest_co + 1;

    preempt_prim(&MAC_CCCH_REQ_PRIM, C_TYP_DCCH_DESC, dd_n, NULL, 0, 0);

    lme_rms_obj.CO_successor += dd_n->COL;
    /* while the CO successor more than highest CO, return to 0 and keep counting */
    if (lme_rms_obj.CO_successor > highest_co) lme_rms_obj.CO_successor -= (highest_co + 1);
}

void trans_cc_mac_timer_func(void *args) {
    // trans_cc_sd_timer_func(NULL);

    cc_mac_bd_t *mac_n = malloc(sizeof(cc_mac_bd_t));
    mac_n->c_type = C_TYP_CC_MAC;
    mac_n->mac_len = SEC_MACLEN_64;
    mac_n->mac = init_buffer_ptr(get_sec_maclen(SEC_MACLEN_64));

    preempt_prim(&MAC_CCCH_REQ_PRIM, C_TYP_CC_MAC, mac_n, NULL, 0, 0);
}


void D_SAPC_cb(ld_prim_t *prim) {
    if (prim->prim_seq == DLS_CLOSE_REQ) return;
    /* AS no need to do this callback */

    if (prim->prim_obj_typ == DL_TYP_AS_INIT) {
        return;
    }
    if (prim->prim_err) {
        return;
    }
    dls_en_data_t *en_data = prim->prim_objs;
    cc_cell_resp_t resp = {
            .c_type = C_TYP_CELL_RESP,
            .SAC = en_data->AS_SAC,
            .UA = en_data->AS_UA,
            .PAV = 0,
            .FAV = 0,
            .TAV = 255,
            .CO = get_CO(),
            .EPLDACS = init_buffer_unptr(), /* 16bytes */
            .CCLDACS = 128,
            .VER = 1,
    };

    uint8_t test_epldacs[16] = {0};
    CLONE_TO_CHUNK(*resp.EPLDACS, test_epldacs, EPLDACS_LEN >> 3);

    //初始化一LME_AS结构体，放入map中
    // set_lme_as_enode(get_as_man(resp.SAC, resp.UA, lme_layer_objs.GS_SAC, LD_AUTHC_G0));

    //模拟CO分配
    set_mac_CO(resp.CO, en_data->AS_SAC);

    preempt_prim(&MAC_CCCH_REQ_PRIM, C_TYP_CELL_RESP,
                 gen_pdu(&resp, cc_format_descs[C_TYP_CELL_RESP].f_desc, "cc resp OUT"), NULL, 0, 0);
}


void M_SAPC_L_cb(ld_prim_t *prim) {
    switch (prim->prim_seq) {
        case MAC_CCCH_IND: {
            ld_format_desc_t *desc = &cc_format_descs[prim->prim_obj_typ];
            channel_data_t *channel_data = prim->prim_objs;
            if (channel_data->channel != CC_CHANNEL) {
                prim->prim_err = LD_ERR_WRONG_PARA;
                return;
            }
            void *data_struct = NULL;
            if ((data_struct = parse_sdu(channel_data->buf, desc->f_desc, desc->struct_size)) == NULL) {
                log_warn("Parse SDU failed!");
                desc->free_func(data_struct);
                return;
            }

            switch (prim->prim_obj_typ) {
                case C_TYP_CELL_RESP: {
                    cc_cell_resp_t *resp = data_struct;

                    lme_as_man_t *as_man = lme_rms_obj.lme_obj->lme_as_man;
                    if (resp->UA != as_man->AS_UA) break;

                    /* set the AS SAC and GS SAC of AS LME  */
                    as_man_update_key_handler(as_man, &as_man->AS_SAC, resp->SAC, sizeof(uint16_t), "AS_SAC");
                    as_man_update_key_handler(as_man, &as_man->AS_CURR_GS_SAC, lme_layer_objs.GS_SAC, sizeof(uint16_t),
                                              "GS_SAC");

                    add_co(&as_man->CO, resp->CO);

                    dls_en_data_t *dls_en_data = &(dls_en_data_t) {
                            .GS_SAC = lme_layer_objs.GS_SAC,
                            .AS_UA = resp->UA,
                            .AS_SAC = resp->SAC,
                    };

                    // log_warn("ALLOC CO SAC %d %d", resp->CO, resp->SAC);

                    // init as dls entity ,未来做切换的时候需要考虑可行性
                    preempt_prim(&DLS_OPEN_REQ_PRIM, DL_TYP_AS_INIT, dls_en_data, NULL, 0, 0);

                    lme_layer_objs.finish_status = LME_CONNECTING_FINISHED;
                    break;
                }

                default: {
                    break;
                }
            }
            desc->free_func(data_struct);
            break;
        }
        case MAC_DCCH_IND: {
            ld_format_desc_t *desc = &dc_format_descs[prim->prim_obj_typ];
            channel_data_t *channel_data = prim->prim_objs;
            if (channel_data->channel != DC_CHANNEL) {
                prim->prim_err = LD_ERR_WRONG_PARA;
                return;
            }
            void *data_struct = NULL;
            if ((data_struct = parse_sdu(channel_data->buf, desc->f_desc, desc->struct_size)) == NULL) {
                log_warn("Parse SDU failed!");
                desc->free_func(data_struct);
                return;
            }

            switch (prim->prim_obj_typ) {
                case DC_TYP_KEEP_ALIVE: {
                    dc_keep_alive_t *keep_alive = data_struct;
                    // log_warn("KEEP ALIVE %d", keep_alive->d_type);
                    break;
                }
                case DC_TYP_RSC_RQST: {
                    dc_rsc_rqst_t *rsc_rqst = data_struct;
                    ld_req_update(lme_rms_obj.rl_drr, channel_data->SAC, rsc_rqst->REQ);
                    break;
                }
                case DC_TYP_CELL_EXIT: {
                    dc_cell_exit_t *cell_exit = data_struct;
                    lme_as_man_t *as_man = get_lme_as_enode(cell_exit->SAC);
                    preempt_prim(&DLS_CLOSE_REQ_PRIM, E_TYP_ANY, &cell_exit->SAC, NULL, 0, 0);
                    if (!as_man) {
                        prim->prim_err = LD_ERR_INTERNAL;
                        break;
                    }
                    // config.is_merged == FALSE
                    //     ? trans_gsnf(lme_layer_objs.sgw_conn, &(gsnf_st_chg_t){
                    //                      .G_TYP = GSNF_STATE_CHANGE,
                    //                      .VER = DEFAULT_GSNF_VERSION,
                    //                      .AS_SAC = cell_exit->SAC,
                    //                      .State = GSNF_EXIT,
                    //                      .GS_SAC = lme_layer_objs.GS_SAC
                    //                  }, &gsnf_st_chg_desc, NULL,NULL)
                    //     : trans_gsnf(lme_layer_objs.sgw_conn, &(gsg_as_exit_t){GS_AS_EXIT, cell_exit->SAC},
                    //                  &gsg_as_exit_desc, NULL,NULL);
                    unregister_snf_en(cell_exit->SAC);
                    delete_lme_as_node_by_sac(cell_exit->SAC, clear_as_man);
                    break;
                }
                default:
                    break;
            }

            desc->free_func(data_struct);
            break;
        }
        case MAC_CCCH_REQ: {
            if (prim->prim_obj_typ == C_TYP_CC_MAC) break;
            lme_rms_obj.cc_sz += cc_format_descs[prim->prim_obj_typ].desc_size;
            break;
        }
        case MAC_DCCH_REQ: {
            break;
        }
        default:
            break;
    }
}

void L_SAPR(ld_prim_t *prim) {
    cc_rsc_t *rqst = prim->prim_objs;
    ld_req_update(lme_rms_obj.fl_drr, rqst->SAC, rqst->resource);
    // log_warn("++++++++++++++++++++++++++ %d %d %d", rqst->SAC, rqst->SC, rqst->resource);
}

static void resource_rl_alloc_cb(ld_drr_t *drr, size_t *map, void *args) {
    ld_rpso_t *mac_rpso_sac = args;
    for (int i = 0; i < SAC_MAX; i++) {
        if (map[i] == 0) continue;
        // log_info("\nSAC: %d ;\nALLOCED: %d ;\nDC: %d ; \nREMAIN_REQ: %d;",
        //          drr->req_entitys[i].SAC,
        //          map[i],
        //          drr->req_entitys[i].DC,
        //          drr->req_entitys[i].req_sz);

        /* while map[i] != 0 */
        size_t alloc_blks = ((map[i] - 1) / RL_DATA_BLK_LEN_MAX) + 1;
        if (alloc_blks > mac_rpso_sac->avail_sz) {
            log_fatal("No enough space for rl.");
            return;
        }

        cc_rl_alloc_t rl_alloc = {
                .c_type = C_TYP_RL_ALLOC,
                .SAC = i,
                .RPSO = mac_rpso_sac->avail_start,
                .NRPS = alloc_blks,
                .CMS = CMS_TYP_1,
        };

        preempt_prim(&MAC_CCCH_REQ_PRIM, C_TYP_RL_ALLOC,
                     gen_pdu(&rl_alloc, cc_format_descs[C_TYP_RL_ALLOC].f_desc, "CC RL ALLOC OUT"), NULL, 0, 0);

        mac_rpso_sac->rpsos[mac_rpso_sac->avail_start].SAC = i;
        mac_rpso_sac->rpsos[mac_rpso_sac->avail_start].NRPS = alloc_blks;
        mac_rpso_sac->avail_start += alloc_blks;
        mac_rpso_sac->avail_sz -= alloc_blks;
    }
}

static void resource_fl_alloc_cb(ld_drr_t *drr, size_t *map, void *args) {
    size_t *BO = args;
    for (int i = 0; i < SAC_MAX; i++) {
        if (map[i] == 0) continue;
        // log_info("\nSAC: %d ;\nALLOCED: %d ;\nDC: %d ; \nREMAIN_REQ: %d;",
        //          drr->req_entitys[i].SAC,
        //          map[i],
        //          drr->req_entitys[i].DC,
        //          drr->req_entitys[i].req_sz);

        cc_fl_alloc_t rl_alloc = {
                .c_type = C_TYP_FL_ALLOC,
                .SAC = i,
                .BO = *BO,
                .BL = map[i],
        };

        *BO += map[i];

        if (preempt_prim(&LME_R_IND_PRIM, E_TYP_ANY, &(cc_rsc_t) {.SAC = i, .resource = map[i]}, NULL, 0, 0) != LD_OK) {
            //错误处理
            return;
        }
        preempt_prim(&MAC_CCCH_REQ_PRIM, C_TYP_FL_ALLOC,
                     gen_pdu(&rl_alloc, cc_format_descs[C_TYP_FL_ALLOC].f_desc, "CC FL ALLOC OUT"), NULL, 0, 0);
    }
}
