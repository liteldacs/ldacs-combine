//
// Created by 邹嘉旭 on 2024/2/21.
//
#include "ldacs_phy.h"
#define Nlim 10

static void rl_data_upload(evutil_socket_t fd, short event, void *arg);

static void dc_data_upload(evutil_socket_t fd, short event, void *arg);

static void *activate_phy_mf(void *args);

static void *upload_dc_rl(void *args);

typedef struct phy_slots_s {
    pthread_mutex_t mutex;
    sdu_s_t *sdus;
} phy_slots_t;

typedef struct phy_json_obj_s {
    // int8_t phy_sf_seq;
    phy_layer_objs_t *phy_obj;
    phy_slots_t rl_slots;
    phy_slots_t dc_slots;
    ld_gtimer_t phy_mf_timer;

    gtimer_ev_t gtimer[10];
    stimer_ev_t stimer[10];
} phy_json_obj_t;


static phy_json_obj_t phy_json_obj = {
    .phy_obj = NULL,
    // .phy_sf_seq = 0,
    .rl_slots = {
        .mutex = PTHREAD_MUTEX_INITIALIZER,
    },
    .dc_slots = {
        .mutex = PTHREAD_MUTEX_INITIALIZER,
    },

    .phy_mf_timer = {{.it_interval = {0, MF_TIMER}, .it_value = {0, 0}}},
    .gtimer = {
        {activate_phy_mf, NULL, TIMER_INFINITE},
        {upload_dc_rl, NULL, 4},
    },

    .stimer = {
        {dc_data_upload,NULL, DC_INTVL},
        {rl_data_upload,NULL, RL_INTVL},
    },
};

static void dc_data_upload(evutil_socket_t fd, short event, void *arg) {
    phy_slots_t *slots = &phy_json_obj.dc_slots;

    preempt_prim(&PHY_DC_IND_PRIM, E_TYP_ANY, slots->sdus, NULL, 0, 0);

    CLEAR_BUF_ARRAY_DEEP(slots->sdus->blks, DC_SLOT_MAX);
}

static void rl_data_upload(evutil_socket_t fd, short event, void *arg) {
    phy_slots_t *slots = &phy_json_obj.rl_slots;

    preempt_prim(&PHY_DATA_IND_PRIM, E_TYP_RL, slots->sdus, NULL, 0, 0);

    CLEAR_BUF_ARRAY_DEEP(slots->sdus->blks, RL_SLOT_MAX);
}

static l_err init_dc_data() {
    register_stimer(&phy_json_obj.stimer[0]);
    return LD_OK;
}

static l_err init_rl_data() {
    register_stimer(&phy_json_obj.stimer[1]);
    return LD_OK;
}

static void *activate_phy_mf(void *args) {
    // log_fatal("ACTIVATE PHY");
    register_gtimer(&phy_json_obj.phy_mf_timer);
    register_gtimer_event(&phy_json_obj.phy_mf_timer, &phy_json_obj.gtimer[1]);
    return NULL;
}

static void *upload_dc_rl(void *args) {
    init_dc_data();
    init_rl_data();
    return NULL;
}

typedef void (*handle_upward)(cJSON *);

l_err init_sim_json(phy_layer_objs_t *obj_p) {
    phy_json_obj.phy_obj = obj_p;

    if (config.role == LD_GS) {
        phy_json_obj.dc_slots.sdus = create_sdu_s(DC_SLOT_MAX);
        phy_json_obj.rl_slots.sdus = create_sdu_s(RL_SLOT_MAX);
        register_gtimer_event(&gtimer, &phy_json_obj.gtimer[0]);
    }

    return LD_OK;
}


static void upward_BCCH_j(cJSON *j_node) {
    if (phy_layer_objs.need_sync == TRUE) {
        /* 6.5.3
         * During the initial RL RA access, the AS TX shall directly apply its current
         * FL SF timing for its RL RA transmission, without any timing pre-compensation.*/
        as_init_gtimer();
        phy_layer_objs.need_sync = FALSE;
    }

    bcch_pdu_t bcch_pdu;
    sdu_s_t *sdus = create_sdu_s(BC_BLK_N);
    INIT_BUF_ARRAY_UNPTR(&bcch_pdu, BC_BLK_N);
    unmarshel_json(j_node, (void **) &bcch_pdu, &bcch_j_tmpl_desc);

    if (
        (sdus->blks[0] = decode_b64_buffer(0, bcch_pdu.bc_1->ptr, bcch_pdu.bc_1->total)) == NULL ||
        (sdus->blks[1] = decode_b64_buffer(0, bcch_pdu.bc_2->ptr, bcch_pdu.bc_2->total)) == NULL ||
        (sdus->blks[2] = decode_b64_buffer(0, bcch_pdu.bc_3->ptr, bcch_pdu.bc_3->total)) == NULL) {
        log_warn("BCCH base64 decode failed");
        goto BC_FREE;
    }

    preempt_prim(&PHY_BC_IND_PRIM, E_TYP_ANY, sdus, free_sdu_s, 0, 0);
BC_FREE: {
        CLEAR_BUF_ARRAY_DEEP(&bcch_pdu, BC_BLK_N);
    }
}

static void upward_FL_CC_DCH_j(cJSON *j_node) {
    cc_dch_pdu_t cc_dch_pdu;

    INIT_BUF_ARRAY_UNPTR(&cc_dch_pdu, FL_DATA_BLK_N);
    unmarshel_json(j_node, (void **) &cc_dch_pdu, &cc_dch_j_tmpl_desc);

    buffer_t *cc_dch_bufs[FL_DATA_BLK_N];
    if (
        (cc_dch_bufs[0] = decode_b64_buffer(0, cc_dch_pdu.sdu_1_6->ptr, cc_dch_pdu.sdu_1_6->total)) == NULL ||
        (cc_dch_bufs[1] = decode_b64_buffer(0, cc_dch_pdu.sdu_7_12->ptr, cc_dch_pdu.sdu_7_12->total)) == NULL ||
        (cc_dch_bufs[2] = decode_b64_buffer(0, cc_dch_pdu.sdu_13_21->ptr, cc_dch_pdu.sdu_13_21->total)) == NULL ||
        (cc_dch_bufs[3] = decode_b64_buffer(0, cc_dch_pdu.sdu_22_27->ptr, cc_dch_pdu.sdu_22_27->total)) == NULL) {
        log_warn("FL base64 decode failed");
        goto FL_FREE;
    }; {
        sdu_s_t *sdus_data = create_sdu_s(FL_DATA_BLK_N);

        INIT_BUF_ARRAY_UNPTR(sdus_data->blks, FL_DATA_BLK_N);

        CLONE_BY_BUFFER(*sdus_data->blks[0], *phy_json_obj.phy_obj->cc_dch_merge.data_next_mf[0]);
        CLONE_BY_BUFFER(*sdus_data->blks[1], *phy_json_obj.phy_obj->cc_dch_merge.data_next_mf[1]);
        CLONE_BY_BUFFER(*sdus_data->blks[2], *cc_dch_bufs[0]);
        CLONE_BY_BUFFER(*sdus_data->blks[3], *cc_dch_bufs[1]);

        //CLONE_TO_CHUNK(*phy_json_obj.phy_obj->cc_dch_merge.data_next_mf[0], cc_dch_bufs[2]->ptr + cc_pdu_l, sdus_data->blks[2]->total - cc_pdu_l);
        preempt_prim(&PHY_DATA_IND_PRIM, E_TYP_FL, sdus_data, free_sdu_s, 0, 0);
    }
    /* 8.6.2.5 "The physical layer must forward the received data contained in jointly interleaved PHY-PDUs to
the MAC immediately after PHY-PDU 21."*/
    size_t cc_pdu_l = CC_BLK_LEN_MIN; //use 1 cc-pdu when testing
    {
        sdu_s_t *sdus_cc = create_sdu_s(CC_BLK_N);
        INIT_BUF_ARRAY_UNPTR(sdus_cc->blks, CC_BLK_N);
        CLONE_TO_CHUNK_L(*sdus_cc->blks[0], cc_dch_bufs[2]->ptr, cc_pdu_l, cc_pdu_l);

        preempt_prim(&PHY_CC_IND_PRIM, E_TYP_ANY, sdus_cc, free_sdu_s, 0, 0);
    }

    CLONE_TO_CHUNK(*phy_json_obj.phy_obj->cc_dch_merge.data_next_mf[0], cc_dch_bufs[2]->ptr + cc_pdu_l,
                   cc_dch_bufs[2]->total - cc_pdu_l);
    CLONE_BY_BUFFER(*phy_json_obj.phy_obj->cc_dch_merge.data_next_mf[1], *cc_dch_bufs[3]);

    /* 当decode出问题时， 跳转至FL_FREE处，未来应考虑怎么在接受失败时，刷新所有已缓存内容， attack同理 */
FL_FREE:
    CLEAR_BUF_ARRAY_DEEP(&cc_dch_pdu, FL_DATA_BLK_N);
    CLEAR_BUF_ARRAY_DEEP(&cc_dch_bufs, FL_DATA_BLK_N);
}

static void upward_RACH_j(cJSON *j_node) {
    rach_pdu_t rach_pdu;
    sdu_s_t *sdus = create_sdu_s(RA_BLK_N);

    INIT_BUF_ARRAY_UNPTR(&rach_pdu, RA_BLK_N);
    unmarshel_json(j_node, (void **) &rach_pdu, &rach_j_tmpl_desc);

    if (
        (sdus->blks[0] = decode_b64_buffer(0, rach_pdu.ra_1->ptr, rach_pdu.ra_1->total)) == NULL ||
        (sdus->blks[1] = decode_b64_buffer(0, rach_pdu.ra_2->ptr, rach_pdu.ra_2->total)) == NULL) {
        log_warn("RA base64 decode failed");
        goto RA_FREE;
    }

    preempt_prim(&PHY_RA_IND_PRIM, E_TYP_ANY, sdus, free_sdu_s, 0, 0);

RA_FREE: {
        CLEAR_BUF_ARRAY_DEEP(&rach_pdu, RA_BLK_N);
    }
}

static void upward_DCCH_j(cJSON *j_node) {
    int is_array = cJSON_IsArray(j_node);
    //通过判断jnode是否为数组，进而知道是否为SYNC
    if (!is_array) {
        phy_layer_objs.need_sync = TRUE;
        dcch_sync_t sync;
        unmarshel_json(j_node, (void **) &sync, &dcch_sync_j_tmpl_desc);
        preempt_prim(&PHY_SYNC_IND_PRIM, E_TYP_ANY, &sync.SAC, NULL, 0, 0);
        return;
    }

    size_t array_size = cJSON_GetArraySize(j_node);
    phy_slots_t *slots = &phy_json_obj.dc_slots;

    for (int ind = 0; ind < array_size; ind++) {
        dcch_pdu_t dcch_pdu;
        INIT_BUF_ARRAY_UNPTR(&dcch_pdu.dc, DC_BLK_N);
        cJSON *array_node = cJSON_GetArrayItem(j_node, ind);
        unmarshel_json(array_node, (void **) &dcch_pdu, &dcch_j_tmpl_desc);

        buffer_t *bufs;
        if ((bufs = decode_b64_buffer(0, dcch_pdu.dc->ptr, dcch_pdu.dc->total)) == NULL) {
            log_warn("DC base64 decode failed");
            free_buffer(dcch_pdu.dc);
            CLEAR_BUF_ARRAY_DEEP(&dcch_pdu.dc, DC_BLK_N);
            continue;
        }

        // ld_lock(&slots->mutex);
        slots->sdus->blks[dcch_pdu.slot_ser] = init_buffer_unptr();
        if (bufs->ptr == NULL) {
            log_error("bufs has no data.");
            free_buffer(bufs);
            // ld_unlock(&slots->mutex);
            continue;
        }
        CLONE_TO_CHUNK(*slots->sdus->blks[dcch_pdu.slot_ser], bufs->ptr, DC_BLK_LEN_MAX);
        // ld_unlock(&slots->mutex);

        free_buffer(dcch_pdu.dc);
        free_buffer(bufs);
    }
}

static void upward_RL_DCH_j(cJSON *j_node) {
    size_t array_size = cJSON_GetArraySize(j_node);
    phy_slots_t *slots = &phy_json_obj.rl_slots;

    for (int ind = 0; ind < array_size; ind++) {
        rl_dch_pdu_t rl_dch_pdu;
        INIT_BUF_ARRAY_UNPTR(&rl_dch_pdu.rl_dch, RL_DATA_BLK_N);

        cJSON *array_node = cJSON_GetArrayItem(j_node, ind);
        unmarshel_json(array_node, (void **) &rl_dch_pdu, &rl_dch_j_tmpl_desc);
        // log_error("%s", cJSON_Print(array_node));

        buffer_t *bufs;
        if ((bufs = decode_b64_buffer(0, rl_dch_pdu.rl_dch->ptr, rl_dch_pdu.rl_dch->total)) == NULL) {
            log_warn("RL base64 decode failed");
            free_buffer(rl_dch_pdu.rl_dch);
            CLEAR_BUF_ARRAY_DEEP(&rl_dch_pdu.rl_dch, RL_DATA_BLK_N);
            continue;
        }
        //填充到时隙中
        for (int i = rl_dch_pdu.start; i < rl_dch_pdu.end; i++) {
            slots->sdus->blks[i] = init_buffer_unptr();

            if (bufs->ptr == NULL) {
                log_error("bufs has no data.");
                free_buffer(bufs);
                // ld_unlock(&slots->mutex);
                break;
            }
            CLONE_TO_CHUNK(*slots->sdus->blks[i], bufs->ptr + (RL_DATA_BLK_LEN_MAX * (i - rl_dch_pdu.start)),
                           RL_DATA_BLK_LEN_MAX);
        }

        free_buffer(rl_dch_pdu.rl_dch);
        free_buffer(bufs);
    }
}

static handle_upward handle_upward_funcs[5] = {
    upward_BCCH_j,
    upward_FL_CC_DCH_j,
    upward_RACH_j,
    upward_DCCH_j,
    upward_RL_DCH_j,
};

l_err upward_json_sim(void *data) {
    buffer_t *buf = data;
    phy_json_hdr_t json_hdr;

    cJSON *root = cJSON_Parse((char *) buf->ptr);
    unmarshel_json(root, (void *) &json_hdr, &phy_json_hdr_j_tmpl_desc);
    cJSON *raw_node = cJSON_Parse(json_hdr.raw);

    handle_upward_funcs[json_hdr.channel](raw_node);

    free_buffer(buf);
    free(json_hdr.raw);
    cJSON_Delete(raw_node);
    cJSON_Delete(root);

    return LD_OK;
}

l_err downward_json_sim(ld_prim_t *prim, buffer_t **in_bufs, buffer_t **out_buf) {
    cJSON *j_node = NULL;
    cJSON *root = NULL;
    uint16_t seq = prim->prim_seq;
    phy_json_hdr_t json_hdr = {
        .strength = DEFAULT_SIGNAL_STRENGTH,
    };

    switch (seq) {
        case PHY_BC_REQ: {
            bcch_pdu_t bcch_pdu;
            json_hdr.channel = BCCH;
            //json_hdr.sf_seq = phy_json_obj.phy_sf_seq = 0;

            INIT_BUF_ARRAY_UNPTR(&bcch_pdu, BC_BLK_N);

            clone_by_alloced_buffer(bcch_pdu.bc_1, encode_b64_buffer(0, in_bufs[0]->ptr, in_bufs[0]->total));
            clone_by_alloced_buffer(bcch_pdu.bc_2, encode_b64_buffer(0, in_bufs[1]->ptr, in_bufs[1]->total));
            clone_by_alloced_buffer(bcch_pdu.bc_3, encode_b64_buffer(0, in_bufs[2]->ptr, in_bufs[2]->total));

            j_node = marshel_json(&bcch_pdu, &bcch_j_tmpl_desc);

            CLEAR_BUF_ARRAY_DEEP(&bcch_pdu, BC_BLK_N);

            break;
        }
        case PHY_CC_REQ:
        case PHY_DATA_REQ: {
            // RL link DATA process
            if (prim->prim_obj_typ == E_TYP_RL) {
                json_hdr.channel = RL_DCH;

                j_node = cJSON_CreateArray();
                //ergodic from 0-160 slot
                for (int p = 0; p < RL_SLOT_MAX; p++) {
                    //if slot[p] is null, that means this slot has data
                    if (in_bufs[p] != NULL) {
                        rl_dch_pdu_t rl_dch_pdu;
                        cJSON *array_node = NULL;
                        buffer_t *bufs = init_buffer_ptr(RL_DATA_BLK_LEN_MAX * Nlim);

                        INIT_BUF_ARRAY_UNPTR(&rl_dch_pdu.rl_dch, RL_DATA_BLK_N);
                        // p as start
                        rl_dch_pdu.start = p;
                        // get Nlim(here is 10), or get the last position before being NULL
                        for (; in_bufs[p] != NULL && p - rl_dch_pdu.start < Nlim; p++) {
                            cat_to_buffer(bufs, in_bufs[p]->ptr, in_bufs[p]->total);
                        }
                        rl_dch_pdu.end = p--;

                        change_buffer_len(bufs, bufs->len);

                        clone_by_alloced_buffer(rl_dch_pdu.rl_dch, encode_b64_buffer(0, bufs->ptr, bufs->total));

                        array_node = marshel_json(&rl_dch_pdu, &rl_dch_j_tmpl_desc);
                        CLEAR_BUF_ARRAY_DEEP(&rl_dch_pdu.rl_dch, RL_DATA_BLK_N);


                        cJSON_AddItemToArray(j_node, array_node);
                        free_buffer(bufs);
                    }
                }
                if (cJSON_GetArraySize(j_node) == 0) {
                    goto clean;
                }
                // log_debug("AS PHY OUT");
            } else {
                cc_dch_pdu_t cc_dch_pdu;
                json_hdr.channel = FL_CC_DCH;
                // json_hdr.sf_seq = ++(phy_json_obj.phy_sf_seq);

                INIT_BUF_ARRAY_UNPTR(&cc_dch_pdu, FL_DATA_BLK_N);

                clone_by_alloced_buffer(cc_dch_pdu.sdu_1_6, encode_b64_buffer(0, in_bufs[0]->ptr, in_bufs[0]->total));
                clone_by_alloced_buffer(cc_dch_pdu.sdu_7_12, encode_b64_buffer(0, in_bufs[1]->ptr, in_bufs[1]->total));
                clone_by_alloced_buffer(cc_dch_pdu.sdu_13_21, encode_b64_buffer(0, in_bufs[2]->ptr, in_bufs[2]->total));
                clone_by_alloced_buffer(cc_dch_pdu.sdu_22_27, encode_b64_buffer(0, in_bufs[3]->ptr, in_bufs[3]->total));

                j_node = marshel_json(&cc_dch_pdu, &cc_dch_j_tmpl_desc);

                CLEAR_BUF_ARRAY_DEEP(&cc_dch_pdu, FL_DATA_BLK_N);
            }

            break;
        }
        case PHY_RA_REQ: {
            rach_pdu_t rach_pdu;
            json_hdr.channel = RACH;
            // json_hdr.sf_seq = phy_json_obj.phy_sf_seq = 0;

            INIT_BUF_ARRAY_UNPTR(&rach_pdu, RA_BLK_N);
            clone_by_alloced_buffer(rach_pdu.ra_1, encode_b64_buffer(0, in_bufs[0]->ptr, in_bufs[0]->total));
            clone_by_alloced_buffer(rach_pdu.ra_2, encode_b64_buffer(0, in_bufs[1]->ptr, in_bufs[1]->total));

            j_node = marshel_json(&rach_pdu, &rach_j_tmpl_desc);

            CLEAR_BUF_ARRAY_DEEP(&rach_pdu, RA_BLK_N);

            break;
        }
        case PHY_DC_REQ:
        case PHY_SYNC_REQ: {
            json_hdr.channel = DCCH;
            // json_hdr.sf_seq = 0;
            switch (seq) {
                case PHY_DC_REQ: {
                    j_node = cJSON_CreateArray();
                    for (int p = 0; p < DC_SLOT_MAX; p++) {
                        if (in_bufs[p] != NULL) {
                            dcch_pdu_t dcch_pdu;
                            cJSON *array_node = NULL;

                            dcch_pdu.slot_ser = p;
                            INIT_BUF_ARRAY_UNPTR(&dcch_pdu.dc, DC_BLK_N);
                            clone_by_alloced_buffer(dcch_pdu.dc,
                                                    encode_b64_buffer(0, in_bufs[p]->ptr, in_bufs[p]->total));
                            array_node = marshel_json(&dcch_pdu, &dcch_j_tmpl_desc);
                            CLEAR_BUF_ARRAY_DEEP(&dcch_pdu.dc, DC_BLK_N);

                            cJSON_AddItemToArray(j_node, array_node);
                        }
                    }
                    break;
                }
                case PHY_SYNC_REQ: {
                    j_node = marshel_json(&(dcch_sync_t){.SAC = *(uint16_t *) prim->prim_objs}, &dcch_sync_j_tmpl_desc);
                    break;
                }
            }

            break;
        }
        default: break;
    }

    json_hdr.raw = cJSON_PrintUnformatted(j_node);
    root = marshel_json(&json_hdr, &phy_json_hdr_j_tmpl_desc);
    *out_buf = get_json_buffer(JSON_UNFORMAT, root);

clean:
    free(json_hdr.raw);
    cJSON_Delete(j_node);
    cJSON_Delete(root);

    return LD_OK;
}
