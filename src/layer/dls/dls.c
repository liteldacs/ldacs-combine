//
// Created by 邹嘉旭 on 2024/3/22.
//

#include <iso646.h>

#include "ldacs_dls.h"

l_err process_direct_dls(void *args);

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
    if (config.direct) {
        if ((dls_layer_objs.device = set_device("UDP", process_direct_dls)) == NULL) {
            log_error("Cannot set SNP device");
            return LD_ERR_INTERNAL;
        }

        if (set_new_dls_frequency(config.init_fl_freq, config.init_rl_freq) != LD_OK) {
            return LD_ERR_INTERNAL;
        }

        if (pthread_create(&dls_layer_objs.recv_th, NULL, start_recv, dls_layer_objs.device) != 0) {
            return LD_ERR_THREAD;
        }
        pthread_detach(dls_layer_objs.recv_th);
    }

    return LD_OK;
}

void D_SAPD(ld_prim_t *prim) {
    orient_sdu_t *ori_sdu = dup_prim_data(prim->prim_objs, sizeof(orient_sdu_t));

    if (config.direct) {
        dls_data_t data = {
            .TYP = ACK_DATA,
            .RST = 0,
            .LFR = 0,
            .SC = 0,
            .PID = 0,
            .SEQ2 = 0,
            .DATA = ori_sdu->buf,
        };
        dls_direct_t direct = {
            .AS_SAC = ori_sdu->AS_SAC,
            .GS_SAC = ori_sdu->GS_SAC,
            .dls_pdu = gen_pdu(&data, &dls_data_desc, "DLS DATA DIRECT"),
        };

        buffer_t *out = gen_pdu(&direct, &dls_direct_desc, "DLS DIRECT");

        ld_dev_udp_para_t *upara = (ld_dev_udp_para_t *)dls_layer_objs.device;
        log_warn("!!!!!!!!!! %d", ntohs( upara->fl_send_addr.sin_port));
        dls_layer_objs.device->send_pkt(dls_layer_objs.device, out,
                                        config.role == LD_AS ? RL : FL);

        free_buffer(direct.dls_pdu);
        free_buffer(out);

        return;
    }

    buffer_t *snp_pdu = init_buffer_unptr();
    CLONE_BY_BUFFER(*snp_pdu, *ori_sdu->buf);
    const dls_entity_t *d_entity = NULL;

    switch (config.role) {
        case LD_AS: {
            d_entity = dls_layer_objs.AS_DLS;
            break;
        }
        case LD_GS: {
            d_entity = get_dls_enode(ori_sdu->AS_SAC);
            break;
        }
        default: {
            break;
        }
    }

    if (d_entity == NULL) {
        log_warn("DLS Entity is NULL");
        return;
    }

    switch (prim->prim_obj_typ) {
        case SN_TYP_FROM_LME: {
            lfqueue_put(d_entity->cos_oqueue[DLS_COS_7], snp_pdu);
            break;
        }
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
    if (config.direct) return;
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

            log_warn("=========== %d", rl_data->SAC);

            // log_buf(LOG_INFO, "buf", rl_data->buf->ptr, rl_data->buf->len);
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
                // log_buf(LOG_INFO, "FL INPUT", fl_input->ptr, fl_input->len);
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

                    if (dls_layer_objs.AS_DLS == NULL) {
                        log_warn("AS has not DLS currently");
                        break;
                    }
                    if (rl_alloc->SAC != dls_layer_objs.AS_DLS->AS_SAC) break;

                    // log_warn("!!!!!!!!!=================== %d %d %d", rl_alloc->SAC, rl_alloc->RPSO, rl_alloc->NRPS);

                    set_rl_param(rl_alloc->RPSO, rl_alloc->NRPS);
                    if (dls_frag_func(dls_layer_objs.AS_DLS, rl_alloc->NRPS * RL_DATA_BLK_LEN_MAX) != LD_OK) {
                        log_warn("DLS FRAG FAILED");
                    }
                    break;
                }
                case C_TYP_FL_ALLOC: {
                    cc_fl_alloc_t *fl_alloc = data_struct;
                    // log_warn("========== %d %d", fl_alloc->SAC, lme_layer_objs.lme_as_man->AS_SAC);
                    if (dls_layer_objs.AS_DLS == NULL) {
                        log_warn("AS has not DLS currently");
                        break;
                    }
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
                    if (dls_layer_objs.AS_DLS == NULL) {
                        break;
                    }

                    if (ack->AS_SAC != dls_layer_objs.AS_DLS->AS_SAC) {
                        break;
                    }
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
                    recv_ack(dls_en, ack->PID, ack->bitmap);
                    break;
                }
                case DC_TYP_ACK_FRAG: {
                    dc_frag_ack_t *f_ack = data_struct;
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

l_err process_direct_dls(void *args) {
    buffer_t *buf = args;
    dls_direct_t *direct = calloc(1, sizeof(dls_direct_t));
    PARSE_DSTR_PKT(buf, direct, dls_pdu, dls_direct_desc, 3, 0);
    dls_data_t *data = parse_sdu(direct->dls_pdu, &dls_data_desc, sizeof(dls_data_t));

    if (config.role == LD_AS) {
        if (direct->AS_SAC != lme_layer_objs.lme_as_man->AS_SAC) {
            free_buffer(direct->dls_pdu);
            free(direct);
            return LD_OK;
        }
    }

    orient_sdu_t *o_sdu = create_orient_sdus(direct->AS_SAC, direct->GS_SAC);
    CLONE_TO_CHUNK(*o_sdu->buf, data->DATA->ptr, data->DATA->len);

    free_buffer(direct->dls_pdu);
    free(direct);
    free_buffer(data->DATA);
    free(data);

    l_err err = preempt_prim(&DLS_DATA_IND_PRIM, E_TYP_ANY, o_sdu, free_orient_sdus, 0, 0);
    // free_orient_sdus(o_sdu);
    return err;
}

l_err set_new_dls_frequency(double fl_freq, double rl_freq) {

    if (!set_new_freq(dls_layer_objs.device, fl_freq + 50.0, FL)) {
        log_error("Cannot set new frequency");
        return LD_ERR_INTERNAL;
    }
    if (!set_new_freq(dls_layer_objs.device, rl_freq + 50.0, RL)) {
        log_error("Cannot set new frequency");
        return LD_ERR_INTERNAL;
    }
    return LD_OK;
}

