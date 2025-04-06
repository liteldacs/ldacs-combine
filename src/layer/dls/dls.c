//
// Created by 邹嘉旭 on 2024/3/22.
//
#include "ldacs_dls.h"

dls_layer_objs_t dls_layer_objs = {
};

ld_prim_t DLS_DATA_REQ_PRIM = {
    .name = "DLS_DATA_REQ",
    .prim_seq = DLS_DATA_REQ,
    .SAP = {D_SAPD, NULL, NULL},
    .req_cb = {D_SAPD_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t DLS_DATA_IND_PRIM = {
    .name = "DLS_DATA_IND",
    .prim_seq = DLS_DATA_IND,
    .SAP = {NULL, NULL, NULL},
    .req_cb = {D_SAPD_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t DLS_UDATA_REQ_PRIM = {
    .name = "DLS_UDATA_REQ",
    .prim_seq = DLS_UDATA_REQ,
    .SAP = {D_SAPD, NULL, NULL},
    .req_cb = {NULL, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t DLS_UDATA_IND_PRIM = {
    .name = "DLS_UDATA_IND",
    .prim_seq = DLS_UDATA_IND,
    .SAP = {D_SAPD, NULL, NULL},
    .req_cb = {D_SAPD_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t DLS_OPEN_REQ_PRIM = {
    .name = "DLS_OPEN_REQ",
    .prim_seq = DLS_OPEN_REQ,
    .SAP = {D_SAPC, NULL, NULL},
    .req_cb = {D_SAPC_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

ld_prim_t DLS_CLOSE_REQ_PRIM = {
    .name = "DLS_CLOSE_REQ",
    .prim_seq = DLS_CLOSE_REQ,
    .SAP = {D_SAPC, NULL, NULL},
    .req_cb = {D_SAPC_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};


l_err make_dls_layer() {
    switch (config.role) {
        case LD_AS: {
            init_dls_fsm(&dls_layer_objs, DLS_CLOSED);
            break;
        }
        case LD_GS: {
            init_dls_fsm(&dls_layer_objs, DLS_OPEN);

            /* init dls map, the key is as_sac */
            dls_layer_objs.as_dls_map = dls_init_map();

            break;
        }
        default:
            break;
    }
    return LD_OK;
}

void D_SAPD(ld_prim_t *prim) {
    orient_sdu_t *ori_sdu = dup_prim_data(prim->prim_objs, sizeof(orient_sdu_t));
    buffer_t *snp_pdu = init_buffer_unptr();
    CLONE_BY_BUFFER(*snp_pdu, *ori_sdu->buf);
    const dls_entity_t *d_entity = NULL;

    switch (config.role) {
        case LD_AS:
            d_entity = dls_layer_objs.AS_DLS;
            break;
        case LD_GS: {
            d_entity = get_dls_enode(ori_sdu->AS_SAC);
            break;
        }
        default:
            break;
    }

    if (d_entity == NULL) {
        return;
    }

    switch (prim->prim_obj_typ) {
        case SN_TYP_FROM_LME:
            lfqueue_put(d_entity->cos_oqueue[DLS_COS_7], snp_pdu);
            break;
        case SN_TYP_FROM_UP:
            lfqueue_put(d_entity->cos_oqueue[DLS_COS_7], snp_pdu);
            break;
        default:
            break;
    }
    clear_dup_prim_data(ori_sdu, free);
}

/**
 * Control DLS entity open or not
 */
void D_SAPC(ld_prim_t *prim) {
    switch (prim->prim_seq) {
        case DLS_OPEN_REQ: {
            dls_en_data_t *en_data = prim->prim_objs;
            switch (prim->prim_obj_typ) {
                case DL_TYP_AS_INIT: {
                    dls_layer_objs.AS_DLS = init_dls_entity(en_data->AS_SAC, en_data->GS_SAC, LD_AS);
                    break;
                }
                case DL_TYP_GS_INIT: {
                    set_dls_enode(en_data->GS_SAC, en_data->AS_SAC);
                    break;
                }
                default: {
                    prim->prim_err = LD_ERR_WRONG_PARA;
                }
            }
            break;
        }
        case DLS_CLOSE_REQ: {
            uint16_t as_sac = *(uint16_t *) prim->prim_objs;
            dls_delete_map_node(as_sac, clear_dls_entity);
            break;
        }
        default: {
            prim->prim_err = LD_ERR_INVALID;
            break;
        }
    }
}

void M_SAPD_cb(ld_prim_t *prim) {
    if (prim->prim_seq == MAC_DCH_REQ) return;
    switch (prim->prim_obj_typ) {
        /* GS */
        case D_TYP_RL: {
            channel_data_t *rl_data = prim->prim_objs;

            const dls_entity_t *dls_en;
            if ((dls_en = get_dls_enode(rl_data->SAC)) == NULL) {
                return;
            }

            // log_buf(LOG_ERROR, "buf", rl_data->buf->ptr, rl_data->buf->len);
            buffer_t *rbuffer = init_buffer_unptr();
            CLONE_BY_BUFFER_UNFREE(*rbuffer, *rl_data->buf);
            /* 放入SAC对应实体的重组队列 */
            lfqueue_put(dls_en->reassy_queue, init_queue_node(E_TYP_RL, rbuffer, free_buffer));

            break;
        }
        /* AS */
        case D_TYP_FL: {
            channel_data_t *fl_data = prim->prim_objs;
            if (*fl_data->buf->ptr != 0x00) {
                // log_buf(LOG_DEBUG, "FL INPUT", fl_input->ptr, fl_input->len);
            }

            /* 临时的， 此处以后需要使用MAC状态机以及PHY状态机共同控制 */
            if (dls_layer_objs.AS_DLS == NULL) return;

            buffer_t *fbuffer = init_buffer_unptr();
            CLONE_BY_BUFFER_UNFREE(*fbuffer, *fl_data->buf);
            lfqueue_put(dls_layer_objs.AS_DLS->reassy_queue, init_queue_node(E_TYP_FL, fbuffer, free_buffer));
            break;
        }
        default: {
            break;
        }
    }
}

void M_SAPC_D_cb(ld_prim_t *prim) {
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
                case C_TYP_RL_ALLOC: {
                    cc_rl_alloc_t *rl_alloc = data_struct;

                    set_rl_param(rl_alloc->RPSO, rl_alloc->NRPS);
                    // log_warn("!!!!!!!!!!!!???????????????");
                    dls_frag_func(dls_layer_objs.AS_DLS, rl_alloc->NRPS * RL_DATA_BLK_LEN_MAX);
                    break;
                }
                case C_TYP_FL_ALLOC: {
                    cc_fl_alloc_t *fl_alloc = data_struct;
                    // log_warn("========== %d %d", fl_alloc->SAC, lme_layer_objs.lme_as_man->AS_SAC);
                    if (fl_alloc->SAC == dls_layer_objs.AS_DLS->AS_SAC) {
                        fl_alloc_record_t *record = malloc(sizeof(fl_alloc_record_t));
                        record->BO = fl_alloc->BO;
                        record->BL = fl_alloc->BL;
                        lfqueue_put(mac_layer_objs.fl_alloc_queue, record);
                    }
                    break;
                }
                case C_TYP_ACK: {
                    cc_ack_t *ack = data_struct;
                    if (ack->AS_SAC != dls_layer_objs.AS_DLS->AS_SAC) break;
                    // log_fatal("?????????????????????? %d %d", ack->bitmap, ack->PID);
                    recv_ack(dls_layer_objs.AS_DLS, ack->PID, ack->bitmap);
                    break;
                }
                case C_TYP_ACK_FRAG: {
                    cc_frag_ack_t *f_ack = data_struct;
                    if (f_ack->AS_SAC != dls_layer_objs.AS_DLS->AS_SAC) break;
                    recv_frag_ack(dls_layer_objs.AS_DLS, f_ack->PID, f_ack->SEQ1);
                    break;
                }

                default: break;
            }
            desc->free_func(data_struct);
        }
        case MAC_DCCH_IND: {
            ld_format_desc_t *desc = &dc_format_descs[prim->prim_obj_typ];
            channel_data_t *channel_data = prim->prim_objs;
            dls_entity_t *dls_en;
            void *data_struct = NULL;

            if (channel_data->channel != DC_CHANNEL) {
                prim->prim_err = LD_ERR_WRONG_PARA;
                return;
            }

            if ((dls_en = (dls_entity_t *) get_dls_enode(channel_data->SAC)) == NULL) {
                return;
            }

            if ((data_struct = parse_sdu(channel_data->buf, desc->f_desc, desc->struct_size)) == NULL) {
                log_warn("Parse SDU failed!");
                desc->free_func(data_struct);
                return;
            }

            switch (prim->prim_obj_typ) {
                case DC_TYP_ACK: {
                    dc_ack_t *ack = data_struct;
                    log_fatal("?????????????????????? %d %d", ack->bitmap, ack->PID);
                    recv_ack(dls_en, ack->PID, ack->bitmap);
                    break;
                }
                case DC_TYP_ACK_FRAG: {
                    dc_frag_ack_t *f_ack = data_struct;
                    // log_fatal("?????????????????????? %d %d", f_ack->PID, f_ack->SEQ1);
                    recv_frag_ack(dls_en, f_ack->PID, f_ack->SEQ1);
                    break;
                }
                default: {
                    break;
                }
            }
            desc->free_func(data_struct);
        }
        default: {
            break;
        }
    }
}

void L_SAPR_cb(ld_prim_t *prim) {
    if (prim->prim_seq == LME_R_IND) {
        cc_rsc_t *rsc = prim->prim_objs;

        dls_entity_t *dls_en;
        if ((dls_en = (dls_entity_t *) get_dls_enode(rsc->SAC)) == NULL) {
            return;
        }

        if (dls_frag_func(dls_en, rsc->resource) != LD_OK) {
            return;
        }
    }
}
