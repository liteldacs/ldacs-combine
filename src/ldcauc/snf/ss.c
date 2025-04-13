//
// Created by 邹嘉旭 on 2025/4/3.
//

#include "ldcauc/snf.h"
#include "ldcauc/crypto/secure_core.h"
#include "ldcauc/crypto/cipher.h"
#include "ldcauc/crypto/authc.h"
#include "ldcauc/crypto/key.h"
#include "ldcauc/net/net_core.h"
#include <ld_santilizer.h>


/**
 * 生成认证过程第一条报文AUC_RQST
 * @param args 参数
 */
l_err send_auc_rqst(void *args) {
    snf_entity_t *as_man = args;
    auc_rqst_t *auc_rqst = &(auc_rqst_t){
        . S_TYP = AUC_RQST,
        . VER = snf_obj.PROTOCOL_VER,
        . PID = PID_MAC,
        . AS_SAC = as_man->AS_SAC,
        . GS_SAC = as_man->GS_SAC,
        . MAC_LEN = as_man->AUTHC_MACLEN,
        . AUTH_ID = as_man->AUTHC_AUTH_ID,
        . ENC_ID = as_man->AUTHC_ENC_ID,
        . N_1 = init_buffer_unptr(),
    };

    /* 生成随机数N1 */
    uint8_t n_1_str[NONCE_LEN] = {0};
    generate_nrand(n_1_str, NONCE_LEN);
    CLONE_TO_CHUNK(*auc_rqst->N_1, n_1_str, NONCE_LEN)

    return handle_send_msg(auc_rqst, &auc_rqst_desc, as_man, as_man->key_as_sgw_r_h);
}

l_err recv_auc_rqst(buffer_t *buf, snf_entity_t *as_man) {
    auc_rqst_t rqst;
    l_err err;

    pb_stream pbs;
    zero(&pbs);
    init_pbs(&pbs, buf->ptr, buf->len, "IN MSG");

    if (in_struct(&rqst, &auc_rqst_desc, &pbs, NULL) == FALSE) {
        return LD_ERR_INTERNAL;
    }
    if (!pb_in_mac(&pbs, get_sec_maclen(rqst.MAC_LEN), as_man->key_as_sgw_r_h, verify_hmac_uint)) {
        return LD_ERR_INVALID_MAC;
    }

    // as_man_update_key_handler(as_man, &as_man->AUTHC_MACLEN, rqst.MAC_LEN, sizeof(uint8_t), "maclen");
    // as_man_update_key_handler(as_man, &as_man->AUTHC_AUTH_ID, rqst.AUTH_ID, sizeof(uint8_t), "authid");
    // as_man_update_key_handler(as_man, &as_man->AUTHC_ENC_ID, rqst.ENC_ID, sizeof(uint8_t), "encid");

    as_man->AUTHC_MACLEN = rqst.MAC_LEN;
    as_man->AUTHC_AUTH_ID = rqst.AUTH_ID;
    as_man->AUTHC_ENC_ID = rqst.ENC_ID;

    if ((err = change_state(&as_man->auth_fsm, LD_AUTHC_EV_DEFAULT,
                            &(fsm_event_data_t){&ld_authc_fsm_events[LD_AUTHC_G1], as_man}
         )
        )
    ) {
        log_error("cant change state correctly, %d", err);
    }
    return LD_OK;
}


l_err send_auc_resp(void *args) {
    snf_entity_t *as_man = args;

    /* 生成随机数N2 */
    buffer_t *n_2 = init_buffer_ptr(32);
    uint8_t n_2_str[NONCE_LEN] = {0};
    generate_nrand(n_2_str, NONCE_LEN);
    CLONE_TO_CHUNK(*n_2, n_2_str, NONCE_LEN)

    as_man->shared_random = get_auc_sharedinfo_buf(&(auc_sharedinfo_t){
            . MAC_LEN = as_man->AUTHC_MACLEN,
            . AUTH_ID = as_man->AUTHC_AUTH_ID,
            . ENC_ID = as_man->AUTHC_ENC_ID,
            . N_2 = n_2,
            . AS_SAC = as_man->AS_SAC,
            . GS_SAC = as_man->GS_SAC,
            . K_LEN = as_man->AUTHC_KLEN,
        }
    );

    if (generate_auc_kdf(snf_obj.role, as_man->shared_random, &as_man->key_as_sgw_s_h, &as_man->key_as_gs_h,
                         &as_man->key_as_gs_b, as_man->AS_UA, as_man->GS_SAC)) {
        //进入错误状态
        return LD_ERR_INTERNAL;
    }

    auc_resp_t *auc_resp = &(auc_resp_t){
        . S_TYP = AUC_RESP,
        . VER = snf_obj.PROTOCOL_VER,
        . PID = PID_MAC,
        . AS_SAC = as_man->AS_SAC,
        . GS_SAC = as_man->GS_SAC,
        . MAC_LEN = as_man->AUTHC_MACLEN,
        . AUTH_ID = as_man->AUTHC_AUTH_ID,
        . ENC_ID = as_man->AUTHC_ENC_ID,
        . N_2 = n_2,
        . K_LEN = as_man->AUTHC_KLEN,
    };


    return handle_send_msg(auc_resp, &auc_resp_desc, as_man, as_man->key_as_sgw_s_h);
}

l_err recv_auc_resp(buffer_t *buf, snf_entity_t *as_man) {
    auc_resp_t resp;
    l_err err;
    zero(&resp);
    pb_stream pbs;
    zero(&pbs);
    init_pbs(&pbs, buf->ptr, buf->len, "IN MSG");

    if (in_struct(&resp, &auc_resp_desc, &pbs, NULL) == FALSE) {
        return LD_ERR_INTERNAL;
    }

    as_man->shared_random = get_auc_sharedinfo_buf(&(auc_sharedinfo_t){
            . MAC_LEN = resp.MAC_LEN,
            . AUTH_ID = resp.AUTH_ID,
            . ENC_ID = resp.ENC_ID,
            . N_2 = resp.N_2,
            . AS_SAC = resp.AS_SAC,
            . GS_SAC = resp.GS_SAC,
            . K_LEN = resp.K_LEN,
        }
    );

    if (generate_auc_kdf(snf_obj.role, as_man->shared_random, &as_man->key_as_sgw_s_h, &as_man->key_as_gs_h,
                         &as_man->key_as_gs_b, as_man->AS_UA, 0)) {
        //进入错误状态
        return LD_ERR_INTERNAL;
    }

    if (!pb_in_mac(&pbs, get_sec_maclen(resp.MAC_LEN), as_man->key_as_sgw_s_h, verify_hmac_uint)) {
        return LD_ERR_INVALID_MAC;
    }

    as_man->AUTHC_MACLEN = resp.MAC_LEN;
    as_man->AUTHC_AUTH_ID = resp.AUTH_ID;
    as_man->AUTHC_ENC_ID = resp.ENC_ID;

    if ((err = change_state(&as_man->auth_fsm, LD_AUTHC_EV_DEFAULT,
                            &(fsm_event_data_t){&ld_authc_fsm_events[LD_AUTHC_A2], as_man}
         )
        )
    ) {
        log_error("cant change state correctly, %d", err);
        return LD_ERR_INVAL_STATE_REACHED;
    }

    free_buffer(resp.N_2);

    exit_LME_AUTH();

    return LD_OK;
}

l_err send_auc_key_exec(void *args) {
    snf_entity_t *as_man = args;

    /* 生成随机数N3 */
    buffer_t *n_3 = init_buffer_ptr(32);
    uint8_t n_3_str[NONCE_LEN] = {0};
    generate_nrand(n_3_str, NONCE_LEN);
    CLONE_TO_CHUNK(*n_3, n_3_str, NONCE_LEN)

    auc_key_exec_t *auc_key_exec = &(auc_key_exec_t){
        .
        S_TYP = AUC_KEY_EXC,
        .
        VER = snf_obj.PROTOCOL_VER,
        .
        PID = PID_MAC,
        .
        AS_SAC = as_man->AS_SAC,
        .
        GS_SAC = as_man->GS_SAC,
        .
        MAC_LEN = as_man->AUTHC_MACLEN,
        .
        AUTH_ID = as_man->AUTHC_AUTH_ID,
        .
        ENC_ID = as_man->AUTHC_ENC_ID,
        .
        N_3 = n_3,
    };

    return handle_send_msg(auc_key_exec, &auc_key_exec_desc, as_man, as_man->key_as_sgw_s_h);
}


l_err recv_auc_key_exec(buffer_t *buf, snf_entity_t *as_man) {
    auc_key_exec_t key_exec;
    l_err err;
    pb_stream pbs;
    zero(&pbs);
    init_pbs(&pbs, buf->ptr, buf->len, "IN MSG");

    if (in_struct(&key_exec, &auc_key_exec_desc, &pbs, NULL) == FALSE) {
        return LD_ERR_INTERNAL;
    }

    if (!pb_in_mac(&pbs, get_sec_maclen(key_exec.MAC_LEN), as_man->key_as_sgw_s_h, verify_hmac_uint)) {
        return LD_ERR_INVALID_MAC;
    }

    if ((err = change_state(&as_man->auth_fsm, LD_AUTHC_EV_DEFAULT,
                            &(fsm_event_data_t){&ld_authc_fsm_events[LD_AUTHC_G2], as_man}))) {
        log_error("cant change state correctly, %d", err);
    }

    return LD_OK;
}


static l_err generate_auz_info(buffer_t *buf, void *args) {
    uint16_t SAC = *(uint16_t *) args;
    if (!buf) return LD_ERR_NULL;
    buffer_t *auz_buf = gen_pdu(&(gsnf_as_auz_info_t){
                                    . G_TYP = GSNF_AS_AUZ_INFO,
                                    . VER = DEFAULT_GSNF_VERSION,
                                    . AS_SAC = SAC,
                                    . is_legal = 0x01,
                                    . auz_type = 0x0F,
                                }, &gsnf_as_auz_info_desc, "AS AUZ INFO"
    );

    log_buf(LOG_ERROR, "MID FUNC", auz_buf->ptr, auz_buf->len)
    cat_to_buffer(buf, auz_buf->ptr, auz_buf->len);
    free_buffer(auz_buf);

    return LD_OK;
}

l_err finish_auc(void *args) {
    //the auth has done
    log_info("+++++++++++++===== GS AUTH AS OK =====++++++++++++++");
    snf_entity_t *as_man = args;
    buffer_t *sdu = gen_pdu(&(gs_key_trans_t){
                                . key = as_man->key_as_gs_b,
                                . nonce = as_man->shared_random
                            }, &gs_key_trans_desc, "GS KEY"
    );
    if (trans_gsnf(as_man->gs_conn, &(gsnf_pkt_cn_t){
                       GSNF_KEY_TRANS, DEFAULT_GSNF_VERSION, as_man->AS_SAC, ELE_TYP_8, sdu
                   }, &gsnf_pkt_cn_desc, generate_auz_info, &as_man->AS_SAC
    )) {
        log_warn("SGW send GS key failed");
        free_buffer(sdu);
    }
    free_buffer(sdu);

    // if (config.role == LD_AS || config.role == LD_GS) {
    //     if (preempt_prim(&LME_STATE_IND_PRIM, LME_AS_UPDATE, as_man, NULL, 0, 0)) {
    //     }
    // }

    /* TODO: 临时，用于测试密钥更新 */
    // send_key_update_rqst(as_man);

    return LD_OK;
}

l_err send_key_update_rqst(void *args) {
    snf_entity_t *as_man = args;

    /* 生成随机数NONCE */
    buffer_t *nonce = init_buffer_ptr(32);
    uint8_t NONCE_str[NONCE_LEN] = {0};
    generate_nrand(NONCE_str, NONCE_LEN);
    CLONE_TO_CHUNK(*nonce, NONCE_str, NONCE_LEN)

    key_upd_rqst_t *key_upd_rqst = &(key_upd_rqst_t){
        .S_TYP = KEY_UPD_RQST,
        .VER = snf_obj.PROTOCOL_VER,
        .PID = PID_MAC,
        .AS_SAC = as_man->AS_SAC,
        .KEY_TYPE = MASTER_KEY_AS_SGW,
        .SAC_src = as_man->GS_SAC,
        .SAC_dst = as_man->GS_SAC, /* 假设GS没变 */
        .NCC = 10086,
        .NONCE = nonce,
    };

    handle_send_msg(key_upd_rqst, &key_upd_rqst_desc, as_man, as_man->key_as_sgw_s_h);

    free_buffer(nonce);
    return LD_OK;
}

l_err recv_key_update_rqst(buffer_t *buf, snf_entity_t *as_man) {
    key_upd_rqst_t key_upd_rqst;
    pb_stream pbs;
    zero(&pbs);
    init_pbs(&pbs, buf->ptr, buf->len, "IN MSG");

    in_struct(&key_upd_rqst, &key_upd_rqst_desc, &pbs, NULL);
    if (!pb_in_mac(&pbs, get_sec_maclen(as_man->AUTHC_MACLEN), as_man->key_as_sgw_s_h, verify_hmac_uint)) {
        return LD_ERR_INVALID_MAC;
    }

    // log_warn("NEW GS: %d %d %d", key_upd_rqst.AS_SAC, key_upd_rqst.SAC_src, key_upd_rqst.SAC_dst);

    UA_STR(ua_as);
    UA_STR(ua_gs_src);
    UA_STR(ua_gs_dst);
    UA_STR(ua_sgw);
    get_ua_str(10010, ua_as);
    get_ua_str(10086, ua_gs_src);
    get_ua_str(10087, ua_gs_dst);
    get_ua_str(10000, ua_sgw);
    as_update_mkey(ua_sgw, ua_gs_src, ua_gs_dst, ua_as, key_upd_rqst.NONCE, &as_man->key_as_gs_h);

    send_key_update_resp(as_man);
    return LD_OK;
}


l_err send_key_update_resp(void *args) {
    snf_entity_t *as_man = args;
    key_upd_resp_t key_upd_resp = {
        .S_TYP = KEY_UPD_RESP,
        .VER = snf_obj.PROTOCOL_VER,
        .PID = PID_MAC,
        .AS_SAC = as_man->AS_SAC,
        .KEY_TYPE = MASTER_KEY_AS_SGW,
        .SAC_dst = as_man->GS_SAC,
        .NCC = 10086,
    };
    handle_send_msg(&key_upd_resp, &key_upd_resp_desc, as_man, as_man->key_as_sgw_s_h);
    return LD_OK;
}

l_err recv_key_update_resp(buffer_t *buf, snf_entity_t *as_man) {
    key_upd_resp_t key_upd_resp;
    pb_stream pbs;
    zero(&pbs);
    init_pbs(&pbs, buf->ptr, buf->len, "IN MSG");

    in_struct(&key_upd_resp, &key_upd_resp_desc, &pbs, NULL);
    if (!pb_in_mac(&pbs, get_sec_maclen(as_man->AUTHC_MACLEN), as_man->key_as_sgw_s_h, verify_hmac_uint)) {
        return LD_ERR_INVALID_MAC;
    }

    return LD_OK;
}


l_err send_sn_session_est_resp(void *args) {
    snf_entity_t *as_man = args;
    sn_session_est_resp_t est_resp = {
        .SN_TYP = SN_SESSION_EST_RESP,
        .VER = DEFAULT_GSNF_VERSION,
        .PID = 1, //???
        .AS_SAC = as_man->AS_SAC,
        .IP_AS = init_buffer_unptr()
    };

    char ipv6_bin[16] = {0};

    // Convert IPv6 string to binary
    if (inet_pton(AF_INET6, snf_obj.net_opt.addr, ipv6_bin) != 1) {
        log_error("inet_pton");
        return LD_ERR_INTERNAL;
    }

    CLONE_TO_CHUNK(*est_resp.IP_AS, (uint8_t *) ipv6_bin, IPV6_ADDRLEN >> 3)

    handle_send_msg(&est_resp, &sn_session_est_resp_desc, as_man, NULL);

    free_buffer(est_resp.IP_AS);
    return LD_OK;
}

l_err recv_sn_session_est_rqst(buffer_t *buf, snf_entity_t *as_man) {
    sn_session_est_rqst_t est_rqst;
    pb_stream pbs;
    zero(&pbs);
    init_pbs(&pbs, buf->ptr, buf->len, "IN MSG");

    in_struct(&est_rqst, &sn_session_est_rqst_desc, &pbs, NULL);

    // log_error("%d %d", est_rqst.SER_TYPE, est_rqst.AS_SAC);
    send_sn_session_est_resp(as_man);

    return LD_OK;
}

/**
 * 向SNP层传送子网控制报文
 * @param args 待组装结构体
 * @param desc 对应结构体描述，用以组装报文
 * @param as_man 相关AS实体
 */
l_err handle_send_msg(void *args, struct_desc_t *desc, snf_entity_t *as_man, KEY_HANDLE key_med) {
    pb_stream lme_ss_pbs;
    zero(&lme_ss_pbs);
    uint8_t ss_buf[MAX_SNP_SDU_LEN] = {0};
    buffer_t *sdu = init_buffer_unptr();

    /* 组装报文 */
    init_pbs(&lme_ss_pbs, ss_buf, MAX_SNP_SDU_LEN, "LME SS BUF");
    if (!out_struct(args, desc, &lme_ss_pbs, NULL)) return LD_ERR_INTERNAL;

    if (key_med != NULL) {
        /* 计算MAC，并将结果置于报文后面 */
        pb_out_mac(&lme_ss_pbs, get_sec_maclen(as_man->AUTHC_MACLEN), key_med, calc_hmac_uint);
    }
    close_output_pbs(&lme_ss_pbs);

    if (snf_obj.role == LD_SGW) {
        CLONE_TO_CHUNK(*sdu, lme_ss_pbs.start, pbs_offset(&lme_ss_pbs))
        trans_gsnf(as_man->gs_conn,
                   &(gsnf_pkt_cn_t){GSNF_SNF_UPLOAD, DEFAULT_GSNF_VERSION, as_man->AS_SAC, ELE_TYP_F, sdu},
                   &gsnf_pkt_cn_desc, NULL, NULL);
    } else if (snf_obj.role == LD_AS) {
        snf_obj.trans_snp_func(as_man->AS_SAC, as_man->GS_SAC, lme_ss_pbs.start, pbs_offset(&lme_ss_pbs));
    }

    return LD_OK;
}

l_err handle_recv_msg(buffer_t *buf, const snf_entity_t *as_man) {
    if (buf == NULL) return LD_ERR_NULL;

    size_t handler_size = 0;
    ss_recv_handler_t *handler = NULL;
    switch (snf_obj.role) {
        case LD_AS: {
            handler_size = as_recv_handlers_sz;
            handler = as_recv_handlers;
            break;
        }
        case LD_SGW: {
            handler_size = sgw_recv_handlers_sz;
            handler = sgw_recv_handlers;
            break;
        }
        default:
            return LD_ERR_INTERNAL;
    }

    for (int i = 0; i < handler_size; i++) {
        if (handler[i].type == *buf->ptr) {
            return handler[i].callback(buf, (snf_entity_t *) as_man);
        }
    }
    return LD_ERR_WRONG_PARA;
}
