//
// Created by 邹嘉旭 on 2024/3/22.
//


#include "ldacs_dls.h"


static void *lineup_func(void *args);

static l_err dls_resource_request(size_t req_size, DLS_COS cos, void *d_entity);

static void *dls_reassy_func(void *args);

static void *dls_upload_func(void *args);

static l_err init_ack_bitset(ld_bitset_t *bitset) {
    uint8_t *res = malloc(bitset->res_sz * bitset->res_num);
    bitset->resources = res;
    return LD_OK;
}

static void free_ack_bitset(void *res) {
    free(res);
}

static void *acquire_cyc_func(void *args) {
    dls_entity_t *d_en = args;

    for (int i = DLS_COS_7; i >= DLS_COS_0; i--) {
        ld_lock(&d_en->cos_req_mutex[i]);
        if (d_en->cos_req_res[i] != 0) {
            switch (config.role) {
                case LD_AS: {
                    /*send RSC_RQST by DCCH channel*/
                    preempt_prim(&MAC_DCCH_REQ_PRIM, DC_TYP_RSC_RQST,
                                 gen_pdu(&(dc_rsc_rqst_t){
                                             .d_type = DC_TYP_RSC_RQST, .SC = i, .REQ = d_en->cos_req_res[i]
                                         },
                                         dc_format_descs[DC_TYP_RSC_RQST].f_desc, "DC RSC RQST"), NULL, 0,
                                 1);

                    // log_warn("!!!!??????????? RSC RQST SEND %d", d_en->cos_req_res[i]);
                    break;
                }
                case LD_GS: {
                    preempt_prim(&LME_R_REQ_PRIM, E_TYP_ANY, &(cc_rsc_t){
                                     .SAC = d_en->AS_SAC, .SC = i, .resource = d_en->cos_req_res[i]
                                 }, NULL, 0, 0);
                    break;
                }
                default: {
                    break;
                }
            }
            d_en->cos_req_res[i] = 0;
        }
        ld_unlock(&d_en->cos_req_mutex[i]);
    }
    return NULL;
}

static uint16_t get_bitmap(ld_bitset_t *ack_set, int lowest_pos) {
    uint8_t *dst = calloc(BITSET_SIZE(ack_set->res_num), sizeof(uint8_t));
    bit_rightshift(ack_set->bitset, BITSET_SIZE(ack_set->res_num), dst, lowest_pos);

    /* 11.6.1 Binary one %1 in the BITMAP field shall signal the successful reception of the corresponding DLS-SDU. */
    uint16_t ret = dst[1] << 8 | dst[0];
    free(dst);
    return ret;
}

/*
 * Function: ack_cyc_func
 * -----------------------
 * Acknowledges resource acquisition and refreshes the acknowledgment bitset.
 *
 * Parameters:
 *   - args: A pointer to a dls_entity_t structure containing the entity data.
 */
static void *ack_cyc_func(void *args) {
    dls_entity_t *d_en = args;

    do {
        if (d_en->ack_bitset == NULL) {
            break;
        }


        // int highest_pos = bs_get_highest(d_en->ack_bitset);
        int lowest_pos = bs_get_lowest(d_en->ack_bitset);
        int alloced_map = bs_get_alloced(d_en->ack_bitset);

        // log_warn("}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}} %d", bs_get_lowest(d_en->ack_bitset));
        if (lowest_pos >= 0 && alloced_map != 0) {
            uint16_t bitmap = get_bitmap(d_en->ack_bitset, lowest_pos);


            // log_fatal("LOWEST:  %d,  ALLOCED_MAP:  %d,  BITMAP:  %d", lowest_pos, alloced_map, bitmap);
            // log_buf(LOG_FATAL, "BITSET", d_en->ack_bitset->bitset, BITSET_SIZE(DLS_WINDOW_SIZE));
            if (config.role == LD_GS) {
                preempt_prim(&MAC_CCCH_REQ_PRIM, C_TYP_ACK,
                             gen_pdu(&(cc_ack_t){
                                         .c_type = C_TYP_ACK, .PID = lowest_pos,
                                         .bitmap = bitmap, .AS_SAC = d_en->AS_SAC
                                     },
                                     cc_format_descs[C_TYP_ACK].f_desc, "CC ACK"), NULL, 0, 1);
            } else {
                preempt_prim(&MAC_DCCH_REQ_PRIM, DC_TYP_ACK,
                             gen_pdu(&(dc_ack_t){
                                         .d_type = DC_TYP_ACK, .PID = lowest_pos,
                                         .bitmap = bitmap
                                     },
                                     dc_format_descs[DC_TYP_ACK].f_desc, "DC ACK"), NULL, 0, 1);
            }
        }

        // fresh the bitset
        free_bitset(d_en->ack_bitset);
    } while (0);
    d_en->ack_bitset = init_bitset(DLS_WINDOW_SIZE, sizeof(uint8_t), init_ack_bitset, free_ack_bitset);
    return NULL;
}


dls_entity_t *init_dls_entity(uint16_t src_sac, uint16_t dest_sac, ldacs_roles role) {
    dls_entity_t *dls_entity = malloc(sizeof(dls_entity_t));
    dls_entity->AS_SAC = role == LD_AS ? src_sac : dest_sac;
    dls_entity->GS_SAC = role == LD_AS ? dest_sac : src_sac;
    dls_entity->role = role;


    dls_entity->reassy_queue = lfqueue_init();
    for (int i = 0; i < 8; i++) {
        dls_entity->cos_iqueue[i] = lfqueue_init();
        dls_entity->cos_oqueue[i] = lfqueue_init();
        pthread_mutex_init(&dls_entity->cos_req_mutex[i], NULL);
    }

    dls_entity->iwindow = init_window(DLS_WINDOW_SIZE);
    dls_entity->owindow = init_window(DLS_WINDOW_SIZE);

    memset(dls_entity->cos_req_res, 0, 8 * sizeof(size_t));

    dls_entity->ack_bitset = NULL;
    dls_entity->req_timer = (ld_gtimer_t){{.it_interval = {0, MF_TIMER}, .it_value = {0, 0}}};
    register_gtimer(&dls_entity->req_timer);
    register_gtimer_event(&dls_entity->req_timer, &(gtimer_ev_t){acquire_cyc_func, dls_entity, TIMER_INFINITE});
    register_gtimer_event(&dls_entity->req_timer, &(gtimer_ev_t){ack_cyc_func, dls_entity, TIMER_INFINITE});

    pthread_create(&dls_entity->trans_wait_th, NULL, lineup_func, dls_entity);
    pthread_create(&dls_entity->recv_th, NULL, dls_reassy_func, dls_entity);
    pthread_create(&dls_entity->upload_th, NULL, dls_upload_func, dls_entity);

    pthread_detach(dls_entity->trans_wait_th);
    pthread_detach(dls_entity->recv_th);
    pthread_detach(dls_entity->upload_th);

    return dls_entity;
}

l_err clear_dls_entity(dls_entity_t *en) {
    if (en->reassy_queue) lfqueue_destroy(en->reassy_queue);
    for (int i = 0; i < 8; i++) {
        lfqueue_destroy(en->cos_iqueue[i]);
        lfqueue_destroy(en->cos_oqueue[i]);
    }
    window_destory(en->iwindow);
    window_destory(en->owindow);

    unregister_gtimer(&en->req_timer);
    if (en->trans_wait_th) pthread_cancel(en->trans_wait_th);
    if (en->recv_th) pthread_cancel(en->recv_th);
    if (en->upload_th) pthread_cancel(en->upload_th);

    return LD_OK;
}

dls_entity_t *init_dls_as_entity(uint16_t gs_sac, uint16_t as_sac) {
    dls_entity_t *as_entity = init_dls_entity(gs_sac, as_sac, LD_GS);
    return as_entity;
}

/**
 * 按优先级从各个队列拿到首个最重要数据包
 * @param d_en
 * @param cos
 * @param buf
 * @return
 */
static l_err get_cos_pkts(dls_entity_t *d_en, uint8_t *cos, buffer_t **buf) {
    for (int i = DLS_COS_7; i >= DLS_COS_0; i--) {
        if (lfqueue_size(d_en->cos_oqueue[i]) == 0) continue;
        lfqueue_get(d_en->cos_oqueue[i], (void **) buf);
        if (*buf != NULL) {
            *cos = i;
            return LD_OK;
        }
    }
    return LD_ERR_NULL;
}

/**
 * 窗口排队线程
 * @param args
 * @return
 */
static void *lineup_func(void *args) {
    dls_entity_t *d_entity = args;
    window_t *w = d_entity->owindow;
    while (stop_flag == FALSE) {
        // if (w->avail_size == 0) pthread_cond_wait(w->put_cond, w->put_mutex);
        buffer_t *obuf = NULL;
        uint8_t cos;
        if (get_cos_pkts(d_entity, &cos, &obuf) != LD_OK) {
            usleep(1000);
            continue;
        }
        uint8_t pos = 0;
        if (window_put(w, cos, obuf, &pos) != LD_OK) {
            continue;
        }


        /*  resource acquire when put into window
        * 11.2.3 Whenever the state of one or more transmission buffers or queues has changed, the resource
        * acquisition function of the AS DLS shall compute the total amount of needed resources for each service class.
        */

        /* TODO： 重传记录obuf，重传资源申请也是obuf长度+DLS_HEAD_LEN,ld_window只需要保证是否被ACK，不关心重传 */
        uint8_t win_end = window_end(w);
        // if ((win_end < w->to_send_start && pos < win_end) || (pos >= w->to_send_start && pos < win_end) || (
        //         win_end < w->to_send_start && pos >= w->to_send_start)) {
        //     log_fatal("============ !???");
        //     dls_resource_request(obuf->len + DATA_HEAD_LEN, cos, d_entity);
        // }

        // 更清晰的无效区域检测
        int is_valid = (win_end >= w->to_send_start)
                           ? (pos >= w->to_send_start && pos < win_end)
                           : (pos < win_end || pos >= w->to_send_start);

        // log_fatal("position: toack=%u, winsz=%u, seqsz=%u, pos=%u, win_end=%u, to_send_start=%u",
        //           w->to_ack_start, w->win_size, w->seq_sz, pos, win_end, w->to_send_start);
        if (is_valid) {
            dls_resource_request(obuf->len + DATA_HEAD_LEN, cos, d_entity);
        } else {
            log_fatal("position: pos=%u, win_end=%u, to_send_start=%u",
                      pos, win_end, w->to_send_start);
        }

        free_buffer(obuf);
    }
    return NULL;
}

static l_err dls_resource_request(size_t req_size, DLS_COS cos, void *d_entity) {
    dls_entity_t *d_en = d_entity;

    ld_lock(&d_en->cos_req_mutex[cos]);
    d_en->cos_req_res[cos] += req_size;
    ld_unlock(&d_en->cos_req_mutex[cos]);
    return LD_OK;
}

l_err recv_ack(dls_entity_t *en, uint8_t PID, uint16_t bitmap) {
    uint8_t normal_endian = ((bitmap >> BITS_PER_BYTE) & 0xFF) << BITS_PER_BYTE | (bitmap & 0xFF);
    // int offset = 1;
    int offset = 0;
    while (TRUE) {
        if ((normal_endian >> offset) & 0x01) {
            // log_error("!!!!!!!!!!!!!!!!!!!!!!!!!");
            // window_ack_item(en->owindow, PID + offset - 1);
            window_ack_item(en->owindow, PID + offset);
        }
        if (normal_endian >> offset == 0) break;
        offset++;
    }
    return LD_OK;
}

l_err recv_frag_ack(dls_entity_t *en, uint8_t PID, uint16_t SEQ1) {
    return LD_OK;
}


l_err dls_frag_func(dls_entity_t *en, size_t alloced) {
    if (!en) return LD_ERR_NULL;
    window_t *win = en->owindow;
    window_ctx_t *w_ctx = NULL;
    int64_t alloced_sz = (int64_t) alloced;
    buffer_t *mac_sdu = init_buffer_ptr(alloced_sz);


    for (int i = 0; i < win->win_size; i++) {
        if ((alloced_sz -= DATA_HEAD_LEN) < 0) {
            break;
        }

        if ((w_ctx = window_check_get(win, &alloced_sz)) == NULL) {
            // log_fatal("windows items is NULL");
            break;
        }
        // log_buf(LOG_INFO, "items[i]", w_ctx->buf->ptr, w_ctx->buf->len);

        pb_stream pbs;
        zero(&pbs);

        dls_data_t dls_data = {
            .TYP = ACK_DATA,
            .RST = w_ctx->is_rst,
            .LFR = w_ctx->is_lfr,
            .SC = w_ctx->cos,
            .PID = w_ctx->pid,
            .SEQ2 = w_ctx->offset,
            .DATA = w_ctx->buf,
        };

        uint8_t out_stream[MAX_INPUT_BUFFER_SIZE] = {0};
        init_pbs(&pbs, out_stream, sizeof(out_stream), "D_SAPD out");
        out_struct(&dls_data, &dls_data_desc, &pbs, NULL);
        close_output_pbs(&pbs);

        cat_to_buffer(mac_sdu, pbs.start, pbs_offset(&pbs));

        // log_warn("************  %d", dls_data.SEQ2);
        free_window_ctx(w_ctx);
    }

    dls_data_req_t *dls_data_req = create_dls_data_req(0, mac_sdu);
    preempt_prim(&MAC_DCH_REQ_PRIM, E_TYP_ANY, dls_data_req, NULL, 0, 0);

    return LD_OK;
}

static void *dls_reassy_func(void *args) {
    dls_entity_t *d_entity = args;
    while (stop_flag == FALSE) {
        ld_queue_node_t *q_node = NULL;
        lfqueue_get_wait(d_entity->reassy_queue, (void **) &q_node);
        if (q_node == NULL) continue;

        buffer_t *rbuf = q_node->n_data;

        // log_buf(LOG_FATAL, "buf", rbuf->ptr, rbuf->len);

        pb_stream pbs;
        zero(&pbs);
        init_pbs(&pbs, rbuf->ptr, rbuf->len, "REASSY INPUT");

        //以当前首位是否为0作为判断结束之标准， TODO：判断方法是临时的
        while (*pbs.start != 0x00) {
            dls_data_t dls_data;
            // dls_data.DATA = init_buffer_unptr();

            if (in_struct(&dls_data, &dls_data_desc, &pbs, NULL) == FALSE) {
                free_buffer(dls_data.DATA);
                break;
            }

            //TODO: 内存泄漏
            window_ctx_t ctx = {
                .pid = dls_data.PID,
                .cos = dls_data.SC,
                .is_lfr = dls_data.LFR,
                .is_rst = dls_data.RST,
                .offset = dls_data.SEQ2,
                .buf = init_buffer_unptr(),
            };

            CLONE_TO_CHUNK(*ctx.buf, dls_data.DATA->ptr, dls_data.DATA->len);

            // log_warn("!!!^^^^^^^^^^^ %d %d  ", d_entity->iwindow->to_recv_start, dls_data.PID);
            l_err err = window_put_ctx(d_entity->iwindow, &ctx);
            if (err == LD_OK) {
                if (ctx.is_lfr) {
                    /* send ACK by CCCH channel */
                    bs_record_by_index(d_entity->ack_bitset, ctx.pid);
                    // log_warn("========= %d", bs_get_lowest(d_entity->ack_bitset));
                } else {
                    preempt_prim(&MAC_CCCH_REQ_PRIM, C_TYP_ACK_FRAG,
                                 gen_pdu(&(cc_frag_ack_t){
                                             .c_type = C_TYP_ACK_FRAG, .PID = ctx.pid,
                                             .SEQ1 = ctx.offset, .AS_SAC = d_entity->AS_SAC
                                         },
                                         cc_format_descs[C_TYP_ACK_FRAG].f_desc, "CC FRAG ACK"), NULL, 0, 1);
                }
            } else {
                log_error("Cannot input");
            }

            free_buffer(dls_data.DATA);
            //置下一start为当前cur，继续解析
            pbs.start = pbs.cur;
        }
        free_queue_node(q_node);
    }
    return NULL;
}

static void *dls_upload_func(void *args) {
    dls_entity_t *d_entity = args;
    while (stop_flag == FALSE) {
        buffer_t *up_buf = window_in_get(d_entity->iwindow);
        if (!up_buf) {
            usleep(10000);
            continue;
        }

        orient_sdu_t *o_sdu = create_orient_sdus(d_entity->AS_SAC, d_entity->GS_SAC);
        CLONE_BY_BUFFER(*o_sdu->buf, *up_buf);
        preempt_prim(&DLS_DATA_IND_PRIM, E_TYP_ANY, o_sdu, free_orient_sdus, 0, 0);

        free_buffer(up_buf);
    }
    return NULL;
}
