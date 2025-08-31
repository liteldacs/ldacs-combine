//
// Created by 邹嘉旭 on 2023/12/14.
//

#include "ldacs_snp.h"
// #include "ldcauc/ldcauc.h"
#include "ldcauc.h"

#include <layer_interface.h>
#include <ldacs_def.h>
#include <snp_sub.h>

l_err process_direct(void *args);

l_err process_snp(orient_sdu_t *o_sdu);

const enum_names snp_pdu_ctrl_names = {USER_PLANE_PACKET, CONTROL_PLANE_PACKET, snp_ctrl_name, NULL};
const enum_names snp_pdu_sec_names = {SEC_MACLEN_INVAILD, SEC_MACLEN_256, snp_sec_name, NULL};

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

    // if (config.direct) {
    //     if ((snp_layer_objs.device = set_device("UDP", process_direct)) == NULL) {
    //         log_error("Cannot set SNP device");
    //         return LD_ERR_INTERNAL;
    //     }
    //
    //     if (set_new_snp_frequency(config.init_fl_freq, config.init_rl_freq) != LD_OK) {
    //         return LD_ERR_INTERNAL;
    //     }
    //
    //     if (pthread_create(&snp_layer_objs.recv_th, NULL, start_recv, snp_layer_objs.device) != 0) {
    //         return LD_ERR_THREAD;
    //     }
    //     pthread_detach(snp_layer_objs.recv_th);
    // }

    return LD_OK;
}


static uint32_t *get_SQN(uint16_t AS_SAC, bool is_send) {
    lme_as_man_t *as_man = config.role == LD_AS ? lme_layer_objs.lme_as_man : (lme_as_man_t *) get_lme_as_enode(AS_SAC);

    if (as_man == NULL) {
        log_error("Cannot find AS: %d", as_man->AS_SAC);
        return NULL;
    }

    return is_send ? &as_man->send_T_SQN : &as_man->recv_T_SQN;
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

    // log_fatal("^^^ TO SEND: %d", orient_sdu_from->AS_SAC);
    orient_sdu_t *orient_sdu_to = create_orient_sdus(orient_sdu_from->AS_SAC, orient_sdu_from->GS_SAC);
    buffer_t *buf = orient_sdu_from->buf;

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

    uint8_t enc_arr[2048] = {0};
    size_t enc_sz = 0;
    buffer_t *enc_buf = init_buffer_unptr();
    if (snpsub_crypto(orient_sdu_to->AS_SAC, buf->ptr, buf->len, enc_arr, &enc_sz, TRUE) != LDCAUC_OK) {
        free_buffer(enc_buf);
        prim->prim_err = LD_ERR_INTERNAL;
        return;
    }
    // log_buf(LOG_INFO, "TO DEC", enc_arr, enc_sz);
    CLONE_TO_CHUNK(*enc_buf, enc_arr, enc_sz);

    /* 如果还没有派生KAS-GS，则不验证完整性 */
    snp_pdu_t snp_pdu = {
        .ctrl = prim->prim_obj_typ == SN_TYP_FROM_UP ? USER_PLANE_PACKET : CONTROL_PLANE_PACKET,
        .sec_level = snp_layer_objs.SEC,
        .nsel = prim->prim_obj_typ == SN_TYP_FROM_UP ? NSEL_IPV6 : NSEL_LME,
        .sdu = enc_buf,
        .sqn = (*get_SQN(orient_sdu_to->AS_SAC, TRUE))++,
    };

    init_pbs(&snp_pbs, snp_buf, MAX_SNP_SDU_LEN, "SNP BUF");
    if (!out_struct(&snp_pdu, &snp_pdu_desc, &snp_pbs, NULL)) {
        prim->prim_err = LD_ERR_INTERNAL;
        free_buffer(enc_buf);
        return;
    }

    char hmac[32] = {0};
    size_t hmac_len = 0;
    if (snpsub_calc_hmac(orient_sdu_to->AS_SAC, snp_layer_objs.SEC, snp_pbs.start, pbs_offset(&snp_pbs),
                         hmac, &hmac_len) != LDCAUC_OK) {
        log_warn("SNP PDU calculate HMAC failed.");
        close_output_pbs(&snp_pbs);
        clear_dup_prim_data(orient_sdu_from, free);
        free_buffer(enc_buf);
    }
    // log_buf(LOG_INFO, "TO CALC MAC", snp_pbs.start, pbs_offset(&snp_pbs));
    memcpy(snp_pbs.cur, hmac, hmac_len);
    snp_pbs.cur += hmac_len;
    close_output_pbs(&snp_pbs);

    CLONE_TO_CHUNK(*orient_sdu_to->buf, snp_pbs.start, pbs_offset(&snp_pbs));
    // log_buf(LOG_INFO, "SNP OUT", orient_sdu_to->buf->ptr, orient_sdu_to->buf->len);

    preempt_prim(&DLS_DATA_REQ_PRIM, prim->prim_obj_typ, orient_sdu_to, free_orient_sdus, 0, 0);

    // if (config.direct) {
    //     snp_direct_t direct = {
    //         .AS_SAC = orient_sdu_to->AS_SAC,
    //         .GS_SAC = orient_sdu_to->GS_SAC,
    //         .snp_pdu = init_buffer_unptr(),
    //     };
    //
    //
    //     CLONE_TO_CHUNK(*direct.snp_pdu, snp_pbs.start, pbs_offset(&snp_pbs));
    //     snp_layer_objs.device->send_pkt(snp_layer_objs.device, gen_pdu(&direct, &snp_direct_desc, "SNP DIRECT"),
    //                                     config.role == LD_AS ? RL : FL);
    //     free_buffer(direct.snp_pdu);
    // } else {
    //     preempt_prim(&DLS_DATA_REQ_PRIM, prim->prim_obj_typ, orient_sdu_to, free_orient_sdus, 0, 0);
    // }


    free_buffer(enc_buf);
    clear_dup_prim_data(orient_sdu_from, free);
}

void D_SAPD_cb(ld_prim_t *prim) {
    switch (prim->prim_seq) {
        case DLS_DATA_IND:
        case DLS_UDATA_IND: {
            orient_sdu_t *o_sdu = prim->prim_objs;
            prim->prim_err = process_snp(o_sdu);
            return;
        }
        case DLS_DATA_REQ:
        case DLS_UDATA_REQ: {
            break;
        }
        default:
            break;
    }
}

// l_err process_direct(void *args) {
//     buffer_t *buf = args;
//     snp_direct_t *direct = calloc(1, sizeof(snp_direct_t));
//     PARSE_DSTR_PKT(buf, direct, snp_pdu, snp_direct_desc, 3, 0);
//
//     // log_fatal("^^^ HAS RECV: %d", direct->AS_SAC);
//
//     if (config.role == LD_AS) {
//         if (direct->AS_SAC != lme_layer_objs.lme_as_man->AS_SAC) {
//             free_buffer(direct->snp_pdu);
//             free(direct);
//             return LD_OK;
//         }
//     }
//
//     // log_warn("?????????/ %d %d", direct->AS_SAC, direct->GS_SAC);
//
//
//     orient_sdu_t *o_sdu = create_orient_sdus(direct->AS_SAC, direct->GS_SAC);
//     CLONE_TO_CHUNK(*o_sdu->buf, direct->snp_pdu->ptr, direct->snp_pdu->len);
//
//     free_buffer(direct->snp_pdu);
//     free(direct);
//
//     const l_err err = process_snp(o_sdu);
//     free_orient_sdus(o_sdu);
//     return err;
// }
//
l_err process_snp(orient_sdu_t *o_sdu) {
    buffer_t *snp_in = o_sdu->buf;
    pb_stream pbs;
    zero(&pbs);
    snp_pdu_t pdu;

    // log_buf(LOG_FATAL, "SNP IN", snp_in->ptr, snp_in->len);


    /* TODO: 搞出更多的错误代码，然后再网关显示 */
    if (snpsub_vfy_hmac(o_sdu->AS_SAC, snp_layer_objs.SEC, snp_in->ptr, snp_in->len) != LDCAUC_OK) {
        log_warn("HMAC verify failed");
        preempt_prim(&SN_DATA_IND_PRIM, VER_WRONG_MAC, o_sdu, NULL, 0, 0);
        return LD_ERR_INVALID_MAC;
    }

    pdu.sdu = init_buffer_ptr(snp_in->len - get_sec_maclen(snp_layer_objs.SEC) - (SNP_HEAD_LEN >> 3));
    init_pbs(&pbs, snp_in->ptr, snp_in->len, "SNP IN");
    if (!in_struct(&pdu, &snp_pdu_desc, &pbs, NULL)) {
        log_warn("Cant deserialize");
        free_buffer(pdu.sdu);
        return LD_ERR_INTERNAL;
    }

    uint32_t *check_sqn = get_SQN(o_sdu->AS_SAC, FALSE);
    if (check_sqn == NULL) {
        return LD_ERR_INTERNAL;
    }
    if (abs((int) pdu.sqn - *check_sqn) < SNP_RANGE) {
        (*check_sqn) = pdu.sqn;
    } else {
        log_warn("The received sqn is out of range.");
        preempt_prim(&SN_DATA_IND_PRIM, VER_WRONG_SQN, o_sdu, NULL, 0, 0);
        free_buffer(pdu.sdu);
        return LD_ERR_INVALID;
    }

    /* free the previous orient buffer, and set the new one */
    free_buffer(o_sdu->buf);

    uint8_t dec_arr[2048] = {0};
    size_t dec_sz = 0;
    o_sdu->buf = init_buffer_unptr();

    // log_buf(LOG_INFO, "TO DEC", pdu.sdu->ptr, pdu.sdu->len);
    if (snpsub_crypto(o_sdu->AS_SAC, pdu.sdu->ptr, pdu.sdu->len, dec_arr, &dec_sz, FALSE) != LDCAUC_OK) {
        free_buffer(pdu.sdu);
        return LD_ERR_INTERNAL;
    }
    CLONE_TO_CHUNK(*(o_sdu->buf), dec_arr, dec_sz);

    free_buffer(pdu.sdu);
    if (pdu.ctrl == CONTROL_PLANE_PACKET) {
        preempt_prim(&SN_DATA_IND_PRIM, VER_PASS, o_sdu, NULL, 0, 0);
    } else {
        preempt_prim(&SN_DATA_IND_PRIM, VER_PASS, o_sdu, NULL, 0, 1);
    }
    return LD_OK;
}
