//
// Created by 邹嘉旭 on 2023/12/14.
//

#include "ldacs_snp.h"

#include <layer_interface.h>
#include <ldacs_def.h>

enum_names snp_pdu_ctrl_names = {USER_PLANE_PACKET, CONTROL_PLANE_PACKET, snp_ctrl_name, NULL};
enum_names snp_pdu_sec_names = {SEC_MACLEN_INVAILD, SEC_MACLEN_256, snp_sec_name, NULL};

static snp_layer_objs_t snp_layer_objs = {
    .SNP_P_SDU = MAX_SNP_SDU_LEN, /* 2048 octets - HEADER_LEN(1 octet) - SQN(3 octet) - MAX_MAC(256bits(32 octet)) */
    .SEC = SEC_MACLEN_256,
    .T_SQN = 0,
};

ld_prim_t SN_DATA_REQ_PRIM = {
    .name = "SN_DATA_REQ",
    .prim_seq = SN_DATA_REQ,
    .SAP = {SN_SAPD, NULL, NULL},
    .req_cb = {SN_SAPD_L_cb, SN_SAPD_U_cb, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t SN_DATA_IND_PRIM = {
    .name = "SN_DATA_IND",
    .prim_seq = SN_DATA_IND,
    .SAP = {NULL, NULL, NULL},
    .req_cb = {SN_SAPD_L_cb, SN_SAPD_U_cb, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t SN_UDATA_REQ_PRIM = {
    .name = "SN_UDATA_REQ",
    .prim_seq = SN_UDATA_REQ,
    .SAP = {SN_SAPD, NULL, NULL},
    .req_cb = {SN_SAPD_L_cb, SN_SAPD_U_cb, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t SN_UDATA_IND_PRIM = {
    .name = "SN_UDATA_IND",
    .prim_seq = SN_UDATA_IND,
    .SAP = {NULL, NULL, NULL},
    .req_cb = {SN_SAPD_L_cb, SN_SAPD_U_cb, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t SN_OPEN_REQ_PRIM = {
    .name = "SN_OPEN_REQ",
    .prim_seq = SN_OPEN_REQ,
    .SAP = {SN_SAPC, NULL, NULL},
    .req_cb = {SN_SAPC_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t SN_AUTH_REQ_PRIM = {
    .name = "SN_AUTH_REQ",
    .prim_seq = SN_AUTH_REQ,
    .SAP = {SN_SAPC, NULL, NULL},
    .req_cb = {SN_SAPC_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t SN_CLOSE_REQ_PRIM = {
    .name = "SN_CLOSE_REQ",
    .prim_seq = SN_CLOSE_REQ,
    .SAP = {SN_SAPC, NULL, NULL},
    .req_cb = {SN_SAPC_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};
ld_prim_t SN_CONF_REQ_PRIM = {
    .name = "SN_CONF_REQ",
    .prim_seq = SN_CONF_REQ,
    .SAP = {SN_SAPC, NULL, NULL},
    .req_cb = {SN_SAPC_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

l_err make_snp_layer() {
    switch (config.role) {
        case LD_AS: {
            init_snp_fsm(&snp_layer_objs, SNP_CLOSED);
            break;
        }
        case LD_GS: {
            init_snp_fsm(&snp_layer_objs, SNP_OPEN);
            break;
        }
        default: {
            break;
        }
    }
    return LD_OK;
}

static KEY_HANDLE get_hmac_key(uint16_t AS_SAC) {
    lme_as_man_t *as_man = config.role == LD_AS ? lme_layer_objs.lme_as_man : (lme_as_man_t *) get_lme_as_enode(AS_SAC);
    return as_man->key_as_gs_h;
}

static uint32_t *get_SQN(uint16_t AS_SAC, bool is_send) {
    lme_as_man_t *as_man = config.role == LD_AS ? lme_layer_objs.lme_as_man : (lme_as_man_t *) get_lme_as_enode(AS_SAC);

    return is_send ? &as_man->send_T_SQN : &as_man->recv_T_SQN;
}

static bool lme_finish_auth(uint16_t AS_SAC) {
    lme_as_man_t *as_man = config.role == LD_AS
                               ? lme_layer_objs.lme_as_man
                               : (lme_as_man_t *) get_lme_as_enode(AS_SAC);
    switch (config.role) {
        case LD_AS:
        case LD_SGW: {
            const char *authc_str = config.role == LD_AS
                                        ? ld_authc_fsm_states[LD_AUTHC_A2]
                                        : ld_authc_fsm_states[LD_AUTHC_G2];
            return in_state(&as_man->auth_fsm, authc_str);
        }
        case LD_GS: {
            return as_man->gs_finish_auth;
        }
        default: {
            return FALSE;
        }
    }
}

void SN_SAPC(ld_prim_t *prim) {
    switch (prim->prim_seq) {
        case SN_AUTH_REQ: {
            /* check out if the current state is connecting */
            if (!in_state(&snp_layer_objs.snp_fsm, snp_fsm_states[SNP_CLOSED])) {
                prim->prim_err = LD_ERR_WRONG_STATE;
                break;
            }

            /* change to the new state auth */
            if ((prim->prim_err = change_state(&snp_layer_objs.snp_fsm, SNP_EV_DEFAULT,
                                               &(fsm_event_data_t){&snp_fsm_events[SNP_AUTH], NULL}))) {
                log_error("cant change state correctly");
                break;
            }
            break;
        }
        case SN_OPEN_REQ: {
            /* check out if the current state is connecting */
            if (!in_state(&snp_layer_objs.snp_fsm, snp_fsm_states[SNP_AUTH])) {
                prim->prim_err = LD_ERR_WRONG_STATE;
                break;
            }

            /* change to the new state auth */
            if ((prim->prim_err = change_state(&snp_layer_objs.snp_fsm, SNP_EV_DEFAULT,
                                               &(fsm_event_data_t){&snp_fsm_events[SNP_OPEN], NULL}))) {
                log_error("cant change state correctly");
                break;
            }
            break;
        }
        default: {
            break;
        }
    }
}

void SN_SAPD(ld_prim_t *prim) {
    if (in_state(&snp_layer_objs.snp_fsm, snp_fsm_states[SNP_CLOSED])) {
        prim->prim_err = LD_ERR_WRONG_STATE;
        return;
    }
    //refuse any user data when snp state is SN_AUTH
    if (in_state(&snp_layer_objs.snp_fsm, snp_fsm_states[SNP_AUTH]) && prim->prim_obj_typ == SN_TYP_FROM_UP) {
        prim->prim_err = LD_ERR_WRONG_STATE;
        return;
    }

    orient_sdu_t *orient_sdu_from = dup_prim_data(prim->prim_objs, sizeof(orient_sdu_t));
    orient_sdu_t *orient_sdu_to = create_orient_sdus(orient_sdu_from->AS_SAC, orient_sdu_from->GS_SAC);
    buffer_t *buf = orient_sdu_from->buf;

    /* 判断LME是否处于已经完成KAS-GS密钥协商阶段 */
    bool finish_auth = lme_finish_auth(orient_sdu_to->AS_SAC);

    pb_stream snp_pbs;
    zero(&snp_pbs);
    uint8_t snp_buf[MAX_SNP_SDU_LEN] = {0};

    switch (prim->prim_seq) {
        case SN_DATA_REQ: {
            break;
        }
        case SN_UDATA_REQ: {
            break;
        }
        default:
            break;
    }

    /* 如果还没有派生KAS-GS，则不验证完整性 */
    snp_pdu_t snp_pdu = {
        .ctrl = prim->prim_obj_typ == SN_TYP_FROM_UP ? USER_PLANE_PACKET : CONTROL_PLANE_PACKET,
        .sec_level = finish_auth ? snp_layer_objs.SEC : SEC_MACLEN_INVAILD,
        .nsel = prim->prim_obj_typ == SN_TYP_FROM_UP ? NSEL_IPV6 : NSEL_LME,
        .sdu = buf,
        .sqn = (*get_SQN(orient_sdu_to->AS_SAC, TRUE))++,
    };

    init_pbs(&snp_pbs, snp_buf, MAX_SNP_SDU_LEN, "SNP BUF");
    if (!out_struct(&snp_pdu, &snp_pdu_desc, &snp_pbs, NULL)) {
        prim->prim_err = LD_ERR_INTERNAL;
    }

    if (finish_auth) {
        // buffer_t *asgs_key = get_hmac_key(orient_sdu_to->AS_SAC);
        // // log_warn("!!!!!!!!!!!!!!!!!! %d", pdu_len);
        // log_buf(LOG_FATAL, "AS GS KEY", asgs_key->ptr, asgs_key->len);
        pb_out_mac(&snp_pbs, get_sec_maclen(snp_layer_objs.SEC), get_hmac_key(orient_sdu_to->AS_SAC), calc_hmac_uint);
    }
    close_output_pbs(&snp_pbs);

    CLONE_TO_CHUNK(*orient_sdu_to->buf, snp_pbs.start, pbs_offset(&snp_pbs));
    log_buf(LOG_INFO, "SNP OUT", orient_sdu_to->buf->ptr, orient_sdu_to->buf->len);
    preempt_prim(&DLS_DATA_REQ_PRIM, prim->prim_obj_typ, orient_sdu_to, free_orient_sdus, 0, 0);

    clear_dup_prim_data(orient_sdu_from, free);
}

void D_SAPD_cb(ld_prim_t *prim) {
    switch (prim->prim_seq) {
        case DLS_DATA_IND:
        case DLS_UDATA_IND: {
            orient_sdu_t *o_sdu = prim->prim_objs;
            buffer_t *snp_in = o_sdu->buf;
            pb_stream pbs;
            zero(&pbs);
            snp_pdu_t pdu;

            uint32_t pdu_len = snp_in->len - (SNP_ELES_SIZE >> 3);

            /* 判断LME是否处于已经完成KAS-GS密钥协商阶段 */
            bool finish_auth = lme_finish_auth(o_sdu->AS_SAC);
            if (finish_auth) {
                /* 应减去MAC长度 */
                pdu_len -= get_sec_maclen(snp_layer_objs.SEC);

                // buffer_t *asgs_key = get_hmac_key(o_sdu->AS_SAC);
                // // log_warn("!!!!!!!!!!!!!!!!!! %d", pdu_len);
                // log_buf(LOG_FATAL, "AS GS KEY", asgs_key->ptr, asgs_key->len);
            }

            pdu.sdu = init_buffer_ptr(pdu_len);

            init_pbs(&pbs, snp_in->ptr, snp_in->len, "SNP IN");

            log_buf(LOG_WARN, "SNP IN", snp_in->ptr, snp_in->len);

            if (!in_struct(&pdu, &snp_pdu_desc, &pbs, NULL)) {
                prim->prim_err = LD_ERR_INTERNAL;
                return;
            }


            /* TODO: 搞出更多的错误代码，然后再网关显示 */
            if (finish_auth) {
                if (!pb_in_mac(&pbs, get_sec_maclen(snp_layer_objs.SEC), get_hmac_key(o_sdu->AS_SAC),
                               verify_hmac_uint)) {
                    free_buffer(o_sdu->buf);
                    o_sdu->buf = pdu.sdu;
                    preempt_prim(&SN_DATA_IND_PRIM, VER_WRONG_MAC, o_sdu, NULL, 0, 0);
                    prim->prim_err = LD_ERR_INVALID_MAC;
                    return;
                }
            }

            uint32_t *check_sqn = get_SQN(o_sdu->AS_SAC, FALSE);
            if (abs((int) pdu.sqn - *check_sqn) < SNP_RANGE) {
                (*check_sqn) = pdu.sqn;
                // log_fatal("SQQNNNN %d %d", pdu.sqn, *check_sqn);
            } else {
                log_warn("The received sqn is out of range.");
                return;
            }

            /* free the previous orient buffer, and set the new one */
            free_buffer(o_sdu->buf);
            o_sdu->buf = pdu.sdu;

            if (pdu.ctrl == CONTROL_PLANE_PACKET) {
                preempt_prim(&SN_DATA_IND_PRIM, VER_PASS, o_sdu, NULL, 0, 0);
            } else {
                preempt_prim(&SN_DATA_IND_PRIM, VER_PASS, o_sdu, NULL, 0, 1);
            }
            break;
        }
        case DLS_DATA_REQ:
        case DLS_UDATA_REQ: {
            break;
        }
        default:
            break;
    }
}
