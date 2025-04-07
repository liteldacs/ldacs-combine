//
// Created by 邹嘉旭 on 2024/1/14.
//

#include "ldacs_lme.h"

typedef struct lme_ss_obj_s {
    lme_layer_objs_t *lme_obj;
} lme_ss_obj_t;

static lme_ss_obj_t lme_ss_obj = {
    .lme_obj = NULL,
};


l_err init_lme_ss(lme_layer_objs_t *obj) {
    lme_ss_obj.lme_obj = obj;

    switch (config.role) {
        case LD_AS:
            break;
        case LD_GS:
            break;
        default:
            break;
    }
    return LD_OK;
}


void SN_SAPC_cb(ld_prim_t *prim) {
}

// static buffer_t *gen_failed_pkt(enum ELE_TYP failed_type, uint16_t as_sac, buffer_t *failed_sdu) {
//     return gen_pdu(&(failed_message_t){
//                        .SN_TYP = FAILED_MESSAGE,
//                        .VER = lme_ss_obj.lme_obj->PROTOCOL_VER,
//                        .PID = PID_RESERVED,
//                        .AS_SAC = as_sac,
//                        .FAILED_TYPE = failed_type,
//                        .msg = failed_sdu,
//                    }, &failed_message_desc, "FAILED MESSAGE");
// }
//

void SN_SAPD_L_cb(ld_prim_t *prim) {
    /* sub-net control will not have unacknowledged data */
    switch (prim->prim_seq) {
        case SN_DATA_IND: {
            orient_sdu_t *osdu = prim->prim_objs;
            buffer_t *in_buf = NULL;
#ifdef HAS_SGW
            if (config.role == LD_GS) {
                lme_as_man_t *as_man = get_lme_as_enode(osdu->AS_SAC);
                if (config.is_merged == TRUE) {
                    // send failed message to SGW
                    if (prim->prim_obj_typ != VER_PASS) {
                        in_buf = gen_failed_pkt(prim->prim_obj_typ, as_man->AS_SAC, osdu->buf);
                        trans_gsnf(lme_layer_objs.sgw_conn, &(gsg_pkt_t){
                                       GS_SNF_DOWNLOAD, as_man->AS_SAC, in_buf
                                   }, &gsg_pkt_desc, NULL, NULL);
                        free_buffer(in_buf);
                    } else {
                        uint8_t type;
                        in_buf = osdu->buf;
                        switch (*in_buf->ptr) {
                            case AUC_RQST: {
                                type = GS_INITIAL_MSG;
                                break;
                            }
                            case SN_SESSION_EST_RESP: {
                                type = GS_UP_DOWNLOAD_TRANSPORT;
                                break;
                            }
                            default: {
                                type = GS_SNF_DOWNLOAD;
                            }
                        }
                        trans_gsnf(lme_layer_objs.sgw_conn, &(gsg_pkt_t){
                                       type, as_man->AS_SAC, in_buf
                                   }, &gsg_pkt_desc, NULL, NULL);
                    }
                } else {
                    in_buf = osdu->buf;
                    if (prim->prim_obj_typ != VER_PASS) {
                        in_buf = gen_failed_pkt(prim->prim_obj_typ, as_man->AS_SAC, osdu->buf);
                        trans_gsnf(lme_layer_objs.sgw_conn, &(gsnf_pkt_cn_t){
                                       GSNF_SNF_DOWNLOAD, DEFAULT_GSNF_VERSION, osdu->AS_SAC, prim->prim_obj_typ, in_buf
                                   }, &gsnf_pkt_cn_desc, NULL, NULL);
                        free_buffer(in_buf);
                    } else {
                        if (as_man->gsnf_count++ == 0) {
                            trans_gsnf(lme_layer_objs.sgw_conn, &(gsnf_pkt_cn_ini_t){
                                           GSNF_INITIAL_AS, DEFAULT_GSNF_VERSION, osdu->AS_SAC, ELE_TYP_F,
                                           as_man->AS_UA,
                                           in_buf
                                       }, &gsnf_pkt_cn_ini_desc, NULL,NULL);
                        } else {
                            trans_gsnf(lme_layer_objs.sgw_conn, &(gsnf_pkt_cn_t){
                                           GSNF_SNF_DOWNLOAD, DEFAULT_GSNF_VERSION, osdu->AS_SAC, ELE_TYP_F, in_buf
                                       }, &gsnf_pkt_cn_desc, NULL,NULL);
                        }
                    }
                }
            } else if (config.role == LD_AS) {
#endif
                const lme_as_man_t *as_man = lme_layer_objs.lme_as_man;
                in_buf = osdu->buf;
                if (as_man == NULL) return;

                handle_recv_msg(in_buf, as_man);
#ifdef HAS_SGW
            }
#endif
            break;
        }
        case SN_DATA_REQ: {
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

        if ((err = change_state(&lme_ss_obj.lme_obj->lme_as_man->auth_fsm, LD_AUTHC_EV_DEFAULT,
                                &(fsm_event_data_t){
                                    &ld_authc_fsm_events[LD_AUTHC_A1], lme_ss_obj.lme_obj->lme_as_man
                                }))) {
            log_error("cant change state correctly, %d", err);
            break;
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
