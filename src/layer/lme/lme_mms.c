//
// Created by 邹嘉旭 on 2024/1/14.
//


#include "ldacs_lme.h"

typedef struct lme_mms_obj_s {
    lme_layer_objs_t *lme_obj;

    stimer_ev_t stimer[10];
} lme_mms_obj_t;

static lme_mms_obj_t lme_mms_obj = {
        .lme_obj = NULL,
        .stimer = {
                {trans_bc_mac_timer_func, NULL, BC_MAC_INTVL}
        },
};

static const char *sib_mod_name[] = {
        "MOD_USER_SPECIFIC",
        "MOD_CELL_SPECIFIC",
};

static const char *sib_cms_name[] = {
        "CMS_TYP_1",
        "CMS_TYP_2",
        "CMS_TYP_3",
        "CMS_TYP_4",
        "CMS_TYP_5",
        "CMS_TYP_6",
        "CMS_TYP_7",
        "CMS_TYP_8",
};


enum_names sib_mod_names = {MOD_USER_SPECIFIC, MOD_CELL_SPECIFIC, sib_mod_name, NULL};
enum_names sib_cms_names = {CMS_TYP_1, CMS_TYP_8, sib_cms_name, NULL};


#define BC_DELAY_TIME 500000


l_err start_mms() {
    switch (config.role) {
        case LD_AS:
            register_gtimer_event(&gtimer, &lme_mms_obj.lme_obj->gtimer[2]);
            break;
        case LD_GS:
            register_gtimer_event(&gtimer, &lme_mms_obj.lme_obj->gtimer[0]);
            break;
        default:
            break;
    }
    return LD_OK;
}

l_err init_lme_mms(lme_layer_objs_t *obj) {
    lme_mms_obj.lme_obj = obj;

    // switch (config.role) {
    //     case LD_AS:
    //         register_timer_event(&lme_mms_obj.lme_obj->cyc_def[LME_TIMER_RA_CR]);
    //         break;
    //     case LD_GS:
    //         register_timer_event(&lme_mms_obj.lme_obj->cyc_def[LME_TIMER_BC_TEST]);
    //
    //         break;
    //     default:
    //         break;
    // }
    //
    return LD_OK;
}

void *trans_lme_bc_timer_func(void *args) {
    register_stimer(&lme_mms_obj.stimer[0]);

    register_gtimer(&lme_layer_objs.cc_timer);
    register_gtimer_event(&lme_layer_objs.cc_timer, &lme_layer_objs.gtimer[1]);


    trans_bc_acb_func(NULL);
    trans_bc_sib_func(NULL);
    return NULL;
}

void *trans_ra_cr_timer_func(void *args) {
    //only for LME_CONNECTING state
    if (!in_state(&lme_mms_obj.lme_obj->lme_fsm, lme_fsm_states[LME_CONNECTING])) {
        return NULL;
    }
    ra_cell_rqst_t rqst = {
            .r_type = R_TYP_CR,
            .UA = lme_layer_objs.lme_as_man->AS_UA,
            .SAC = lme_layer_objs.GS_SAC,
            .SCGS = 1,
            .VER = 0x1,
    };

    preempt_prim(&MAC_RACH_REQ_PRIM, R_TYP_CR, gen_pdu(&rqst, ra_format_descs[R_TYP_CR].f_desc, "RA OUT"),
                 NULL, 0, 0);

    return NULL;
}

void trans_bc_acb_func(void *args) {
    bc_acb_t acb_n = {
            .b_type = B_TYP_ACB,
            .SAC = lme_mms_obj.lme_obj->GS_SAC,
            .FLF = lme_mms_obj.lme_obj->init_flf,
            .RLF = lme_mms_obj.lme_obj->init_rlf,
    };

    preempt_prim(&MAC_BCCH_REQ_PRIM, B_TYP_ACB, gen_pdu(&acb_n, bc_format_descs[B_TYP_ACB].f_desc, "ACB_OUT"), NULL, 0,
                 0);
}


void trans_bc_sib_func(void *args) {
    bc_sib_t sib_n = {
            .b_type = B_TYP_SIB,
            .SAC = lme_layer_objs.GS_SAC,
            .VER = lme_layer_objs.PROTOCOL_VER,
            .FLF = lme_layer_objs.init_flf,
            .RLF = lme_layer_objs.init_rlf,
            .MOD = lme_layer_objs.MOD,
            .CMS = lme_layer_objs.CMS,
            .EIRP = lme_layer_objs.EIRP,
    };

    preempt_prim(&MAC_BCCH_REQ_PRIM, B_TYP_SIB, gen_pdu(&sib_n, bc_format_descs[B_TYP_SIB].f_desc, "SIB_OUT"), NULL, 0,
                 0);
}

void trans_bc_mac_timer_func(evutil_socket_t fd, short event, void *arg) {
    bc_mac_bd_t *mac_n = &(bc_mac_bd_t) {
            .b_type = B_TYP_BC_MAC,
            .mac_len = SEC_MACLEN_64,
            .mac = init_buffer_ptr(get_sec_maclen(SEC_MACLEN_64)),
    };
    preempt_prim(&MAC_BCCH_REQ_PRIM, B_TYP_BC_MAC, mac_n, NULL, 0, 0);
}


void M_SAPB_cb(ld_prim_t *prim) {
    if (prim->prim_seq == MAC_BCCH_IND) {
        ld_format_desc_t *desc = &bc_format_descs[prim->prim_obj_typ];
        channel_data_t *channel_data = prim->prim_objs;
        if (channel_data->channel != BC_CHANNEL) {
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
            case B_TYP_SIB: {
                //如果处于CSCANNING状态，才进行处理
                if (!in_state(&lme_mms_obj.lme_obj->lme_fsm, lme_fsm_states[LME_CSCANNING])) break;
                bc_sib_t *sib = data_struct;
                lme_layer_objs.GS_SAC = sib->SAC;
                lme_layer_objs.PROTOCOL_VER = sib->VER;
                lme_layer_objs.init_flf = sib->FLF;
                lme_layer_objs.init_rlf = sib->RLF;
                lme_layer_objs.MOD = sib->MOD;
                lme_layer_objs.CMS = sib->CMS;
                lme_layer_objs.EIRP = sib->EIRP;

                //the cscanning has done
                // lme_layer_objs.finish_status = LME_CSCANNING_FINISHED;
                change_LME_CONNECTING();
                break;
            }
            default: {
                break;
            }
        }
        desc->free_func(data_struct);
    } else {
        //MAC_BCCH_REQ
    }
}

// int times = 0;

void M_SAPR_cb(ld_prim_t *prim) {
    if (prim->prim_seq == MAC_RACH_IND) {
        ld_format_desc_t *desc = &ra_format_descs[prim->prim_obj_typ];
        channel_data_t *channel_data = prim->prim_objs;
        if (channel_data->channel != RA_CHANNEL) {
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
            case R_TYP_CR: {
                //TODO: 应更改！！！！！
                // if (times++ != 0) return;

                ra_cell_rqst_t *cr = data_struct;
                //判断是否存在该UA的连接
                if (lme_map_has_ua(cr->UA)) {
                    return;
                }

                /* simulate sac alloc procession */
                uint16_t sac = generate_urand(SAC_LEN);
                if (has_lme_as_enode(sac) == FALSE) {
                    set_lme_as_enode(init_as_man(sac, cr->UA, lme_layer_objs.GS_SAC));
                }
                register_snf_en(LD_GS, sac, cr->UA, lme_layer_objs.GS_SAC);
                dls_en_data_t *dls_en_data = &(dls_en_data_t) {
                        .GS_SAC = lme_layer_objs.GS_SAC,
                        .AS_UA = cr->UA,
                        .AS_SAC = sac, //和GSC共同协商分配给AS的SAC 10.6.4.5
                };

                preempt_prim(&DLS_OPEN_REQ_PRIM, DL_TYP_GS_INIT, dls_en_data, NULL, 0, 0);

                break;
            }
            default: {
                break;
            }
        }
        desc->free_func(data_struct);
    } else {
        //MAC_BCCH_REQ
    }
}



