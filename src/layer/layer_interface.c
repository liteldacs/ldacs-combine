//
// Created by 邹嘉旭 on 2024/4/20.
//

#include "layer_interface.h"

#include "inside.h"

int_layer_obj_t int_obj = {};

l_err register_int_handler(void (*handler)(user_msg_t *)) {
    int_obj.msg_handler = handler;
    return LD_OK;
}

void SN_SAPD_U_cb(ld_prim_t *prim) {
    if (prim->prim_seq == SN_DATA_REQ) {
        return;
    }
    orient_sdu_t *o_sdu = prim->prim_objs;
    passert(int_obj.msg_handler != NULL);

    if (config.role == LD_GS) {
        if (config.is_e304) {
            inside_combine_update_user_msg(o_sdu->AS_SAC, o_sdu->buf->ptr, o_sdu->buf->len);
        }
    }

    int_obj.msg_handler(&(user_msg_t){
        .AS_SAC = o_sdu->AS_SAC,
        .GS_SAC = o_sdu->GS_SAC,
        .msg = o_sdu->buf
    });
}

/**
 * Send user data for AS
 * @param data data to send
 * @param sz data length
 * @return
 */
l_err send_user_data_as(uint8_t *data, size_t sz) {
    if (config.role != LD_AS) {
        log_error("Invalid role for sending AS data");
        return LD_ERR_INVALID;
    }
    orient_sdu_t *orient_sdu = create_orient_sdus(lme_layer_objs.lme_as_man->AS_SAC,
                                                  config.role == LD_AS
                                                      ? lme_layer_objs.lme_as_man->AS_CURR_GS_SAC
                                                      : lme_layer_objs.GS_SAC);
    CLONE_TO_CHUNK(*orient_sdu->buf, data, sz);

    preempt_prim(&SN_DATA_REQ_PRIM, SN_TYP_FROM_UP, orient_sdu, free_orient_sdus, 0, 0);
    return LD_OK;
}
