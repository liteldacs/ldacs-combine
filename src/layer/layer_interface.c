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
    if (config.is_e304) {
        inside_combine_update_user_msg(o_sdu->AS_SAC, o_sdu->buf->ptr, o_sdu->buf->len);
    }
    int_obj.msg_handler(&(user_msg_t){
        .AS_SAC = o_sdu->AS_SAC,
        .GS_SAC = o_sdu->GS_SAC,
        .msg = o_sdu->buf
    });
}

/* 对于GS，未来需要维护一个IP <-> SAC的路由表以查询目的AS的SAC，对于AS，AS SAC和目的GS SAC都为已知，因此第三个参数仅用于测试，可删除 */
/**
 * Send user data
 * @param data data to send
 * @param sz data length
 * @param AS_SAC SAC of as. In GS, it is the target SAC, in AS, it is the SAC of itself
 * @return
 */
l_err send_user_data(uint8_t *data, size_t sz, uint16_t AS_SAC) {
    orient_sdu_t *orient_sdu = create_orient_sdus(AS_SAC,
                                                  config.role == LD_AS
                                                      ? lme_layer_objs.lme_as_man->AS_CURR_GS_SAC
                                                      : lme_layer_objs.GS_SAC);
    CLONE_TO_CHUNK(*orient_sdu->buf, data, sz);

    preempt_prim(&SN_DATA_REQ_PRIM, SN_TYP_FROM_UP, orient_sdu, free_orient_sdus, 0, 0);
    return LD_OK;
}
