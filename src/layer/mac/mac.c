//
// Created by jiaxv on 2023/12/5.
//

// #include <ldcauc/crypto/cipher.h>
// #include <ldcauc/crypto/key.h>
#include <crypto/cipher.h>
#include <crypto/key.h>

#include "ldacs_mac.h"

static lyr_desc_t *upper_lyr[] = {
};

static lyr_desc_t *lower_lyr[] = {
};

mac_layer_objs_t mac_layer_objs = {
    .state = MAC_FSCANNING,
    .mac_c_rac = 64,
    // .mac_p_sec = SM3_HMAC,
    .isAPNT = FALSE,
    .hmac_len = SEC_MACLEN_64,

    .cc_order = {
        MAX_ORDER, 1, 2, 2, 3, 3, 3, 4, MAX_ORDER, 5, 6, 7, 8, 8, 9, 10, 11, 8, 1,
        1 //Slot descriptor 0x13
    },
    .dc_priority = {MAX_ORDER, 1, MAX_ORDER, 2, 3, 3, 4, 5, 5, 6, 4, 2},
    .ra_priority = {MAX_ORDER, 1, MAX_ORDER, MAX_ORDER},

    .COS = {.mutex = PTHREAD_MUTEX_INITIALIZER, .value = 0},
    .COM = {.mutex = PTHREAD_MUTEX_INITIALIZER, .value = 0},
    .COL = {.mutex = PTHREAD_MUTEX_INITIALIZER, .value = 0},
    .RPSO = {.mutex = PTHREAD_MUTEX_INITIALIZER, .value = 0},
    .NRPS = {.mutex = PTHREAD_MUTEX_INITIALIZER, .value = 0},
    .CCL = {.mutex = PTHREAD_MUTEX_INITIALIZER, .value = 1},

    // .bc_input = {&mac_layer_objs.bc_q, init_bc_input_queue, queue_wait_while},
    .bc_input = {&mac_layer_objs.bc_q, init_bc_input_queue, lfqueue_wait_while, 1},
    .cc_input = {&mac_layer_objs.cc_pq, init_cc_input_queue, pqueue_wait_while, 2},
    .ra_input = {&mac_layer_objs.ra_q, init_ra_input_queue, lfqueue_wait_while, 3},
    .dc_input = {&mac_layer_objs.dc_pq, init_dc_input_queue, pqueue_wait_while, 4},
    .fl_data_input = {&mac_layer_objs.fl_data_q, init_fl_data_input_queue, lfqueue_wait_while, 5},
    .rl_data_input = {&mac_layer_objs.rl_data_q, init_rl_data_input_queue, lfqueue_wait_while, 6},

    .bc_assq_mutex = PTHREAD_MUTEX_INITIALIZER,
    .cd_assq_mutex = PTHREAD_MUTEX_INITIALIZER,
};


lyr_desc_t mac_desc = {
    .name = "MAC",
    .lyr_funcs = NULL,
    .upper_lyr = upper_lyr,
    .lower_lyr = lower_lyr,
    .lyr_obj = &mac_layer_objs,
};

ld_prim_t MAC_CONNECT_REQ_PRIM = {
    .name = "MAC_CONNECT_REQ",
    .prim_seq = MAC_CONNECT_REQ,
    .SAP = {M_SAPI, NULL, NULL},
    .req_cb = {M_SAPI_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};
ld_prim_t MAC_FSCAN_REQ_PRIM = {
    .name = "MAC_FSCAN_REQ",
    .prim_seq = MAC_FSCAN_REQ,
    .SAP = {M_SAPI, NULL, NULL},
    .req_cb = {M_SAPI_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};
ld_prim_t MAC_CSCAN_REQ_PRIM = {
    .name = "MAC_CSCAN_REQ",
    .prim_seq = MAC_CSCAN_REQ,
    .SAP = {M_SAPI, NULL, NULL},
    .req_cb = {M_SAPI_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};
ld_prim_t MAC_GSCAN_REQ_PRIM = {
    .name = "MAC_GSCAN_REQ",
    .prim_seq = MAC_GSCAN_REQ,
    .SAP = {M_SAPI, NULL, NULL},
    .req_cb = {NULL, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};
ld_prim_t MAC_AUTH_REQ_PRIM = {
    .name = "MAC_AUTH_REQ",
    .prim_seq = MAC_AUTH_REQ,
    .SAP = {M_SAPI, NULL, NULL},
    .req_cb = {M_SAPI_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};
ld_prim_t MAC_OPEN_REQ_PRIM = {
    .name = "MAC_OPEN_REQ_REQ",
    .prim_seq = MAC_OPEN_REQ,
    .SAP = {M_SAPI, NULL, NULL},
    .req_cb = {M_SAPI_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};
ld_prim_t MAC_SYNC_REQ_PRIM = {
    .name = "MAC_SYNC_REQ",
    .prim_seq = MAC_SYNC_REQ,
    .SAP = {M_SAPI, NULL, NULL},
    .req_cb = {NULL, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};
ld_prim_t MAC_HO_REQ_PRIM = {
    .name = "MAC_HO_REQ",
    .prim_seq = MAC_HO_REQ,
    .SAP = {M_SAPI, NULL, NULL},
    .req_cb = {NULL, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};
ld_prim_t MAC_SYNC_IND_PRIM = {
    .name = "MAC_SYNC_IND_REQ",
    .prim_seq = MAC_SYNC_IND,
    .SAP = {M_SAPI, NULL, NULL},
    .req_cb = {NULL, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};
ld_prim_t MAC_CC_STATUS_REQ_PRIM = {
    .name = "MAC_CC_STATUS_REQ",
    .prim_seq = MAC_CC_STATUS_REQ,
    .SAP = {M_SAPI, NULL, NULL},
    .req_cb = {NULL, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};
ld_prim_t MAC_BCCH_REQ_PRIM = {
    .name = "MAC_BCCH_REQ",
    .prim_seq = MAC_BCCH_REQ,
    .SAP = {M_SAPB, NULL, NULL},
    .req_cb = {M_SAPB_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};
ld_prim_t MAC_BCCH_IND_PRIM = {
    .name = "MAC_BCCH_IND",
    .prim_seq = MAC_BCCH_IND,
    .SAP = {NULL, NULL, NULL},
    .req_cb = {M_SAPB_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};
ld_prim_t MAC_RACH_REQ_PRIM = {
    .name = "MAC_RACH_REQ",
    .prim_seq = MAC_RACH_REQ,
    .SAP = {M_SAPR, NULL, NULL},
    .req_cb = {M_SAPR_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};
ld_prim_t MAC_RACH_IND_PRIM = {
    .name = "MAC_RACH_IND",
    .prim_seq = MAC_RACH_IND,
    .SAP = {NULL, NULL, NULL},
    .req_cb = {M_SAPR_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};
ld_prim_t MAC_CCCH_REQ_PRIM = {
    .name = "MAC_CCCH_REQ",
    .prim_seq = MAC_CCCH_REQ,
    .SAP = {M_SAPC, NULL, NULL},
    .req_cb = {M_SAPC_L_cb, M_SAPC_D_cb, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};
ld_prim_t MAC_CCCH_IND_PRIM = {
    .name = "MAC_CCCH_IND",
    .prim_seq = MAC_CCCH_IND,
    .SAP = {NULL, NULL, NULL},
    .req_cb = {M_SAPC_L_cb, M_SAPC_D_cb, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};
ld_prim_t MAC_DCCH_REQ_PRIM = {
    .name = "MAC_DCCH_REQ",
    .prim_seq = MAC_DCCH_REQ,
    .SAP = {M_SAPC, NULL, NULL},
    .req_cb = {M_SAPC_L_cb, M_SAPC_D_cb, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};
ld_prim_t MAC_DCCH_IND_PRIM = {
    .name = "MAC_DCCH_IND",
    .prim_seq = MAC_DCCH_IND,
    .SAP = {NULL, NULL, NULL},
    .req_cb = {M_SAPC_L_cb, M_SAPC_D_cb, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};
ld_prim_t MAC_DCH_REQ_PRIM = {
    .name = "MAC_DCH_REQ",
    .prim_seq = MAC_DCH_REQ,
    .SAP = {M_SAPD, NULL, NULL},
    .req_cb = {M_SAPD_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};
ld_prim_t MAC_DCH_IND_PRIM = {
    .name = "MAC_DCH_IND",
    .prim_seq = MAC_DCH_IND,
    .SAP = {NULL, NULL, NULL},
    .req_cb = {M_SAPD_cb, NULL, NULL},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

static void init_bc_input_queue(const ld_queue_node_t *node) {
    preempt_prim(&MAC_BCCH_IND_PRIM, node->type, node->n_data, NULL, 0, 0);
}

static void init_ra_input_queue(const ld_queue_node_t *node) {
    preempt_prim(&MAC_RACH_IND_PRIM, node->type, node->n_data, NULL, 0, 0);
}

static void init_cc_input_queue(const ld_queue_node_t *node) {
    preempt_prim(&MAC_CCCH_IND_PRIM, node->type, node->n_data, NULL, 0, cc_format_descs[node->type].direct);
}

static void init_dc_input_queue(const ld_queue_node_t *node) {
    preempt_prim(&MAC_DCCH_IND_PRIM, node->type, node->n_data, NULL, 0, dc_format_descs[node->type].direct);
}

static void init_fl_data_input_queue(const ld_queue_node_t *node) {
    preempt_prim(&MAC_DCH_IND_PRIM, node->type, node->n_data, NULL, 0, 0);
}

static void init_rl_data_input_queue(const ld_queue_node_t *node) {
    // log_warn("!!!!!!!!!!!!");
    preempt_prim(&MAC_DCH_IND_PRIM, node->type, node->n_data, NULL, 0, 0);
}

static void process_mac_interv(ld_queue_node_t *node) {
    struct list_head *pos;
    list_for_each(pos, mac_layer_objs.interv_head) {
        mac_interv_t *interv = list_entry(pos, mac_interv_t, lpointer);

        interv->func(node);
    }
}

static void *mac_process_func(void *input_struct) {
    input_struct_t *is = input_struct;
    while (stop_flag == FALSE) {
        ld_queue_node_t *node = is->wait_while(*(void **) is->queue_p);
        if (!node) continue;
        is->process_func(node);
        if (node->type == ELE_TYP_RESERVED || node->n_data == NULL) {
            free(node);
            continue;
        }
        process_mac_interv(node);
        free_queue_node(node);
    }
    return NULL;
}


l_err make_mac_layer() {
    /* temperory */
    UA_STR(ua_as);
    UA_STR(ua_sgw);
    //    if (config.role == LD_AS) {
    //        key_get_handle(config.role, get_ua_str(config.UA, ua_as), get_ua_str(10000, ua_sgw), ROOT_KEY,
    //                       &mac_layer_objs.sm3_key);
    //    } else {
    //        key_get_handle(config.role, get_ua_str(10010, ua_as), get_ua_str(10000, ua_sgw), ROOT_KEY,
    //                       &mac_layer_objs.sm3_key);
    //    }
    //

    mac_layer_objs.interv_head = &mac_layer_objs.intervs.lpointer;
    init_list_head(mac_layer_objs.interv_head);

    mac_layer_objs.bc_q = lfqueue_init();
    mac_layer_objs.cc_pq = pqueue_init(MAX_HEAP, ld_cmp_pri, ld_get_pri, ld_set_pri, ld_get_pos, ld_set_pos);
    mac_layer_objs.dc_pq = pqueue_init(MAX_HEAP, ld_cmp_pri, ld_get_pri, ld_set_pri, ld_get_pos, ld_set_pos);
    mac_layer_objs.ra_q = lfqueue_init();
    mac_layer_objs.fl_data_q = lfqueue_init();
    mac_layer_objs.rl_data_q = lfqueue_init();

    mac_layer_objs.cd_assemed_qs = lfqueue_init();
    mac_layer_objs.bc_assem_qs = lfqueue_init();


    if (config.role == LD_AS) {
        init_mac_fsm(&mac_layer_objs, MAC_FSCANNING);
        pthread_create(&mac_layer_objs.bc_input.th, NULL, mac_process_func, &mac_layer_objs.bc_input);
        pthread_create(&mac_layer_objs.cc_input.th, NULL, mac_process_func, &mac_layer_objs.cc_input);
        pthread_create(&mac_layer_objs.fl_data_input.th, NULL, mac_process_func, &mac_layer_objs.fl_data_input);

        pthread_detach(mac_layer_objs.bc_input.th);
        pthread_detach(mac_layer_objs.cc_input.th);
        pthread_detach(mac_layer_objs.fl_data_input.th);

        mac_layer_objs.fl_alloc_queue = lfqueue_init();
    } else if (config.role == LD_GS) {
        init_mac_fsm(&mac_layer_objs, MAC_OPEN);
        pthread_create(&mac_layer_objs.ra_input.th, NULL, mac_process_func, &mac_layer_objs.ra_input);
        pthread_create(&mac_layer_objs.dc_input.th, NULL, mac_process_func, &mac_layer_objs.dc_input);
        pthread_create(&mac_layer_objs.rl_data_input.th, NULL, mac_process_func, &mac_layer_objs.rl_data_input);

        pthread_detach(mac_layer_objs.ra_input.th);
        pthread_detach(mac_layer_objs.dc_input.th);
        pthread_detach(mac_layer_objs.rl_data_input.th);

        mac_layer_objs.dc_coqueue = lfqueue_init();
        mac_layer_objs.rpso_queue = lfqueue_init();
    } else {
    }

    return LD_OK;
}

void M_SAPI(ld_prim_t *prim) {
    switch (prim->prim_seq) {
        case MAC_FSCAN_REQ: {
            prim->prim_err = preempt_prim(&PHY_FSCAN_REQ_PRIM, E_TYP_ANY, prim->prim_objs, NULL, 0, 0);
            break;
        }
        case MAC_CSCAN_REQ: {
            /* check out if the current state is fscanning */
            if (!in_state(&mac_layer_objs.mac_fsm, mac_fsm_states[MAC_FSCANNING])) {
                prim->prim_err = LD_ERR_WRONG_STATE;
                break;
            }
            if ((prim->prim_err = preempt_prim(&PHY_CSCAN_REQ_PRIM, E_TYP_ANY, NULL, NULL, 0, 0))) {
                log_error("cant indicate phy to `PHY_CSAN`");
                break;
            }

            /* change to the new state cscanning */
            if ((prim->prim_err = change_state(&mac_layer_objs.mac_fsm, MAC_EV_DEFAULT,
                                               &(fsm_event_data_t){&mac_fsm_events[MAC_CSCANNING], NULL}))) {
                log_error("cant change state correctly");
                break;
            }

            break;
        }
        case MAC_GSCAN_REQ: {
            if (!in_state(&mac_layer_objs.mac_fsm, mac_fsm_states[MAC_OPEN])) {
                prim->prim_err = LD_ERR_WRONG_STATE;
                break;
            }

            if ((prim->prim_err = preempt_prim(&PHY_GSCAN_REQ_PRIM, E_TYP_ANY, NULL, NULL, 0, 0))) {
                log_error("cant indicate phy to `PHY_CSAN`");
                break;
            }
            break;
        }
        case MAC_CONNECT_REQ: {
            /* check out if the current state is cscanning */
            if (!in_state(&mac_layer_objs.mac_fsm, mac_fsm_states[MAC_CSCANNING])) {
                prim->prim_err = LD_ERR_WRONG_STATE;
                break;
            }

            /* change to the new state connecting */
            if ((prim->prim_err = change_state(&mac_layer_objs.mac_fsm, MAC_EV_DEFAULT,
                                               &(fsm_event_data_t){&mac_fsm_events[MAC_CONNECTING], NULL}))) {
                log_error("cant change state correctly");
                break;
            }
            break;
        }
        case MAC_AUTH_REQ: {
            /* check out if the current state is connecting */
            if (!in_state(&mac_layer_objs.mac_fsm, mac_fsm_states[MAC_CONNECTING])) {
                prim->prim_err = LD_ERR_WRONG_STATE;
                break;
            }

            /* change to the new state auth */
            if ((prim->prim_err = change_state(&mac_layer_objs.mac_fsm, MAC_EV_DEFAULT,
                                               &(fsm_event_data_t){&mac_fsm_events[MAC_AUTH], NULL}))) {
                log_error("cant change state correctly");
                break;
            }
            break;
        }
        case MAC_OPEN_REQ: {
            if ((prim->prim_err = change_state(&mac_layer_objs.mac_fsm, MAC_EV_DEFAULT,
                                               &(fsm_event_data_t){&mac_fsm_events[MAC_OPEN], NULL}))) {
                log_error("cant change state correctly");
                break;
            }
            break;
        }
        case MAC_HO_REQ: {
            if ((prim->prim_err = change_state(&mac_layer_objs.mac_fsm, MAC_EV_DEFAULT,
                                               &(fsm_event_data_t){&mac_fsm_events[MAC_HO2], NULL}))) {
                log_error("cant change state correctly");
                break;
            }
        }
        case MAC_SYNC_REQ: {
            if ((prim->prim_err = preempt_prim(&PHY_SYNC_REQ_PRIM, E_TYP_ANY, NULL, NULL, 0, 0)) != LD_OK) {
                log_warn("Can not call PHY to SYNC signal");
                break;
            }
            break;
        }
        default: {
            break;
        }
    }
}

void M_SAPB(ld_prim_t *prim) {
    ld_queue_node_t *q_node = malloc(sizeof(ld_queue_node_t));
    zero(q_node);
    q_node->type = prim->prim_obj_typ;

    switch (prim->prim_obj_typ) {
        case B_TYP_BC_MAC: {
            q_node->n_data = dup_prim_data(prim->prim_objs, sizeof(bc_mac_bd_t));
            q_node->free_func = free_bc_mac_bd;
            break;
        }
        default: {
            q_node->n_data = prim->prim_objs;
            q_node->free_func = free_buffer;
            break;
        }
    }

    if (mac_layer_objs.bc_curr_out_q == NULL) {
        mac_layer_objs.bc_curr_out_q = lfqueue_init();
    }

    if (mac_layer_objs.bc_curr_out_q == NULL || lfqueue_put(mac_layer_objs.bc_curr_out_q, q_node) !=
        0) {
        prim->prim_err = LD_ERR_QUEUE;
    }

    if (prim->prim_obj_typ == B_TYP_BC_MAC) {
        if (mac_layer_objs.bc_curr_out_q != NULL) {
            ld_lock(&mac_layer_objs.bc_assq_mutex);
            lfqueue_put(mac_layer_objs.bc_assem_qs, mac_layer_objs.bc_curr_out_q);
            ld_unlock(&mac_layer_objs.bc_assq_mutex);
        }
        mac_layer_objs.bc_curr_out_q = NULL;
    }
}

void M_SAPR(ld_prim_t *prim) {
    ld_queue_node_t *q_node = malloc(sizeof(ld_queue_node_t));
    zero(q_node);
    q_node->type = prim->prim_obj_typ;
    q_node->n_data = prim->prim_objs;
    q_node->free_func = free_buffer;

    if (lfqueue_put(mac_layer_objs.ra_q, q_node)) {
        prim->prim_err = LD_ERR_QUEUE;
    }
}

void M_SAPC(ld_prim_t *prim) {
    ld_queue_node_t *q_node = malloc(sizeof(ld_queue_node_t));
    zero(q_node);
    q_node->type = prim->prim_obj_typ;
    if (q_node->type > C_TYP_SLOT_DESC) {
        log_warn("ERROR C TYPE  %d", q_node->type);
        free_queue_node(q_node);
        return;
    }

    switch (config.role) {
        case LD_GS: {
            switch (prim->prim_obj_typ) {
                case C_TYP_CC_MAC:
                    q_node->pri = cc_format_descs[C_TYP_CC_MAC].pri;
                    q_node->n_data = prim->prim_objs;
                    q_node->free_func = free_cc_mac_bd;
                    break;
                case C_TYP_SLOT_DESC: {
                    cc_slot_desc_t *cc_sd = prim->prim_objs;

                    ld_lock(&mac_layer_objs.CCL.mutex);
                    mac_layer_objs.CCL.value = cc_sd->CCL;
                    ld_unlock(&mac_layer_objs.CCL.mutex);

                    q_node->pri = cc_format_descs[C_TYP_SLOT_DESC].pri;
                    q_node->n_data = gen_pdu(prim->prim_objs, cc_format_descs[C_TYP_SLOT_DESC].f_desc, "CCSD OUT");
                    q_node->free_func = free_buffer;
                    break;
                }
                case C_TYP_DCCH_DESC: {
                    cc_dcch_desc_t *cc_dd = malloc(sizeof(cc_dcch_desc_t));
                    memcpy(cc_dd, prim->prim_objs, sizeof(cc_dcch_desc_t));
                    lfqueue_put(mac_layer_objs.dc_coqueue, cc_dd);

                    q_node->pri = cc_format_descs[C_TYP_DCCH_DESC].pri;
                    q_node->n_data = gen_pdu(prim->prim_objs, cc_format_descs[C_TYP_DCCH_DESC].f_desc, "CCDD OUT");
                    q_node->free_func = free_buffer;
                    break;
                }
                default: {
                    q_node->pri = cc_format_descs[prim->prim_obj_typ].pri;
                    q_node->n_data = prim->prim_objs;
                    q_node->free_func = free_buffer;

                    break;
                }
            }

            /*LME对CC_MAC进行定时，时间略短于MF_TIMER，每激活一次CC就激活一次CC_MAC_TIMER，然后mac层进行判断，一旦接收到下传的是CC_MAC，那么队列放入cc_assem_qs */

            if (mac_layer_objs.cd_asseming_node == NULL) {
                mac_layer_objs.cd_asseming_node = init_cc_dch_trans_node();
            }

            if (mac_layer_objs.cd_asseming_node->cc_out_pq == NULL || pqueue_insert(
                    mac_layer_objs.cd_asseming_node->cc_out_pq, q_node) !=
                0) {
                free_cc_dch_trans_node(mac_layer_objs.cd_asseming_node);
                prim->prim_err = LD_ERR_QUEUE;
                return;
            }

            if (prim->prim_obj_typ == C_TYP_CC_MAC) {
                if (mac_layer_objs.cd_asseming_node != NULL) {
                    ld_lock(&mac_layer_objs.cd_assq_mutex);
                    lfqueue_put(mac_layer_objs.cd_assemed_qs, mac_layer_objs.cd_asseming_node);
                    ld_unlock(&mac_layer_objs.cd_assq_mutex);
                }
                // log_warn("^^^^^^^^^^^^ CC ASSEM QS %d", lfqueue_size(mac_layer_objs.cc_assem_qs));
                mac_layer_objs.cd_asseming_node = NULL;
            }
            break;
        }
        case LD_AS: {
            q_node->pri = dc_format_descs[prim->prim_obj_typ].pri;
            q_node->n_data = prim->prim_objs;
            q_node->free_func = free_buffer;

            if (pqueue_insert(mac_layer_objs.dc_pq, q_node)) {
                prim->prim_err = LD_ERR_QUEUE;
            }

            break;
        }
        default: {
            break;
        }
    }
}

void M_SAPD(ld_prim_t *prim) {
    switch (config.role) {
        case LD_AS: {
            /* the as fill dch tiles according to fixed allocation, only need buffer */
            lfqueue_put(mac_layer_objs.rl_data_q, prim->prim_objs);
            break;
        }
        case LD_GS: {
            /* the gs fill dch slots according to fixed allocation, only need buffer */
            if (!mac_layer_objs.cd_asseming_node) {
                prim->prim_err = LD_ERR_NULL;
                return;
            }
            lfqueue_put(mac_layer_objs.cd_asseming_node->dch_q, prim->prim_objs);
            break;
        }
        default:
            break;
    }
}

/**
 * To generate bcch packet to be transmitted to PHY
 * @return
 */
l_err generate_bc_pkt() {
    ld_queue_node_t *node = NULL;
    lfqueue_t *out_q = NULL;
    if (lfqueue_size(mac_layer_objs.bc_assem_qs) == 0) {
        return LD_ERR_QUEUE;
    }
    out_q = lfqueue_wait_while(mac_layer_objs.bc_assem_qs);

    sdu_s_t *sdus = create_sdu_s(BC_BLK_N);
    sdus->blks[0] = init_buffer_ptr(BC_BLK_LEN_1_3);
    sdus->blks[1] = init_buffer_ptr(BC_BLK_LEN_2);
    sdus->blks[2] = init_buffer_ptr(BC_BLK_LEN_1_3);

    // ld_lock(&mac_layer_objs.bc_assq_mutex);
    // if (lfqueue_size(mac_layer_objs.bc_assem_qs) == 0) {
    //     log_fatal("BC QUEUE SIZE IS 0");
    //     ld_unlock(&mac_layer_objs.bc_assq_mutex);
    //     free_sdu_s(sdus);
    //     return;
    // }
    // lfqueue_get(mac_layer_objs.bc_assem_qs, (void **) &out_q);
    // ld_unlock(&mac_layer_objs.bc_assq_mutex);

    buffer_t *to_trans_bc_1_3 = sdus->blks[0];
    buffer_t *to_trans_bc_2 = sdus->blks[1];
    /* TODO: 当前默认1/3信道报文不超过528字节， 2信道报文不超过1000字节，未来需要实现：
     * 1. 为BC_HMAC预留位置，即，当BC_HMAC被get出来时，1/2/3信道均需预留至少(4 + 256 + 4 + 8) = 272bits的位置供BC_HMAC使用
     * 2. 当信道1满时，再启用信道3，若信道1未满就出现BC_HMAC，那么信道3和信道1的剩余位均置0 */
    while (stop_flag == FALSE) {
        if (lfqueue_size(out_q) == 0) {
            log_warn("BC QUEUE EMPTY");
            free_sdu_s(sdus);
            return LD_ERR_QUEUE;
        }

        lfqueue_get(out_q, (void **) &node);
        if (node == NULL || node->n_data == NULL) continue;


        switch (node->type) {
            case B_TYP_BC_MAC:

                lfqueue_destroy(out_q);
                lfqueue_free(out_q);
                goto BC_Cal_Mac;
            default: {
                buffer_t *buf = node->n_data;

                if (sdus->blks[0]->free - buf->len > BC_BLK_FORCE_REMAIN)
                    to_trans_bc_1_3 = sdus->blks[0];
                else if (sdus->blks[2]->free - buf->len > BC_BLK_FORCE_REMAIN)
                    to_trans_bc_1_3 = sdus->blks[2];
                else
                    log_warn("No available BC space.");

                switch (node->type) {
                    case B_TYP_ACB:
                        cat_to_buffer(to_trans_bc_1_3, buf->ptr, buf->len);
                        break;
                    case B_TYP_SIB:
                        cat_to_buffer(to_trans_bc_2, buf->ptr, buf->len);
                        break;
                    default:
                        break;
                }
                break;
            }
        }

        free_queue_node(node);
    }

    /* 计算MAC并插入buffer中 */
BC_Cal_Mac: {
        // log_info("!!!!!!!!!!!! BC CALC MAC");
        /* determine the block that needs to be filled with HMAC through is APNT */
        int start_blk = mac_layer_objs.isAPNT ? 0 : 1;
        int stop_blk = mac_layer_objs.isAPNT ? 2 : 1;

        for (int i = start_blk; i <= stop_blk; i++) {
            uint8_t out_stream[512] = {0};
            pb_stream pbs;
            if (node == NULL) {
                free_sdu_s(sdus);
                return LD_ERR_NULL;
            }
            const bc_mac_bd_t *mac_n = node->n_data;

            calc_hmac_buffer(sdus->blks[i], mac_layer_objs.sm3_key, mac_n->mac, get_sec_maclen(mac_n->mac_len));

            init_pbs(&pbs, out_stream, sizeof(out_stream), "M_APB out");
            out_struct(mac_n, &bc_hmac_desc, &pbs, NULL);
            close_output_pbs(&pbs);
            cat_to_buffer(sdus->blks[i], pbs.start, pbs_offset(&pbs));

            free_queue_node(node);
        }
    }

    preempt_prim(&PHY_BC_REQ_PRIM, E_TYP_ANY, sdus, free_sdu_s, 0, 0);
    return LD_OK;
}


l_err generate_ra_pkt() {
    ld_queue_node_t *node = NULL;

    if (lfqueue_size(mac_layer_objs.ra_q) == 0) {
        return LD_ERR_QUEUE;
    }
    lfqueue_get(mac_layer_objs.ra_q, (void **) &node);
    if (node == NULL) {
        return LD_ERR_NULL;
    }
    buffer_t *buf = node->n_data;

    sdu_s_t *sdus = create_sdu_s(RA_BLK_N);
    sdus->blks[0] = init_buffer_ptr(RA_BLK_LEN);
    sdus->blks[1] = init_buffer_ptr(RA_BLK_LEN);

    for (int i = 0; i < RA_BLK_N; i++) {
        cat_to_buffer(sdus->blks[i], buf->ptr, buf->len);
    }
    node->free_func(node->n_data);

    preempt_prim(&PHY_RA_REQ_PRIM, E_TYP_ANY, sdus, free_sdu_s, 0, 0);
    return LD_OK;
}

l_err generate_cc_pkt() {
    ld_queue_node_t *node = NULL;
    // cc_dch_trans_node_t *cc_dch_node = NULL;

    ld_lock(&mac_layer_objs.cd_assq_mutex);
    if (lfqueue_size(mac_layer_objs.cd_assemed_qs) == 0) {
        ld_unlock(&mac_layer_objs.cd_assq_mutex);
        log_warn("CC ASSEM QUEUE SIZE IS 0");
        return LD_ERR_QUEUE;
    }
    // log_warn("QUEUE SIZE:   %d", lfqueue_size(mac_layer_objs.cd_assemed_qs));
    lfqueue_get(mac_layer_objs.cd_assemed_qs, (void **) &mac_layer_objs.cd_to_trans_node);
    ld_unlock(&mac_layer_objs.cd_assq_mutex);

    sdu_s_t *sdus = create_sdu_s(CC_BLK_N);
    sdus->blks[0] = init_buffer_ptr(CC_BLK_LEN_MAX);

    while (stop_flag == FALSE) {
        if (pqueue_empty(mac_layer_objs.cd_to_trans_node->cc_out_pq)) {
            log_warn("CC OUT PQUEUE EMPTY");
            free_sdu_s(sdus);
            return LD_ERR_QUEUE;
        }
        pqueue_pop(mac_layer_objs.cd_to_trans_node->cc_out_pq, (void **) &node);
        if (node == NULL || node->n_data == NULL) continue;
        if (node->type == C_TYP_CC_MAC) {
            break;
        }
        // if (node->type == C_TYP_RL_ALLOC) {
        //     log_info("RL ALLOC CC SEND");
        // }
        buffer_t *buf = node->n_data;

        cat_to_buffer(sdus->blks[0], buf->ptr, buf->len);
        free_queue_node(node);
    }

    /* 计算MAC并插入buffer中 */
CC_Cal_Mac: {
        uint8_t out_stream[512] = {0};
        pb_stream pbs;
        if (node == NULL) {
            if (stop_flag == FALSE) {
                log_warn("CC MAC NODE IS NULL");
            }
            free_sdu_s(sdus);
            return LD_ERR_NULL;
        }
        bc_mac_bd_t *mac_n = node->n_data;

        calc_hmac_buffer(sdus->blks[0], mac_layer_objs.sm3_key, mac_n->mac, get_sec_maclen(mac_n->mac_len));


        init_pbs(&pbs, out_stream, sizeof(out_stream), "M_SAPC out");
        out_struct(mac_n, &cc_hmac_desc, &pbs, NULL);
        close_output_pbs(&pbs);
        cat_to_buffer(sdus->blks[0], pbs.start, pbs_offset(&pbs));

        //        log_debug("GEN CC: CCL : %d", (*sdus->blks[0]->ptr) >> 5);

        free_queue_node(node);
    }
CC_END:

    /* 9.3.1.2  "The MAC time framing function shall adjust the size of the CC slot in each MF to the necessary minimum." */
    change_buffer_len(sdus->blks[0], ((sdus->blks[0]->len / CC_BLK_LEN_MIN) + 1) * CC_BLK_LEN_MIN);

    preempt_prim(&PHY_CC_REQ_PRIM, E_TYP_ANY, sdus, free_sdu_s, 0, 0);
    return LD_OK;
}


static bool judge_co(uint16_t co) {
    bool ret = FALSE;
    mac_layer_objs_t obj = mac_layer_objs;
    if (obj.COM.value == 0) return FALSE;
    ld_lock(&obj.COM.mutex);
    ld_lock(&obj.COL.mutex);

    // log_debug("???? %d %d %d %d", co, obj.COM.value, obj.COS.value, obj.COL.value);
    if (co % obj.COM.value >= obj.COS.value && co % obj.COM.value < obj.COS.value + obj.COL.value)
        ret = TRUE;

    ld_unlock(&obj.COM.mutex);
    ld_unlock(&obj.COL.mutex);
    return ret;
}

l_err generate_dc_pkt() {
    sdu_s_t *sdus = create_sdu_s(DC_SLOT_MAX);
    ld_queue_node_t *node = NULL;

    /* 9.1.1.2.4  The MAC sub-layer maps the DCCH into (a dedicated sub-slot of) the DC slot. */
    // ld_lock(&mac_layer_objs.CO.mutex);
    ld_lock(&lme_layer_objs.lme_as_man->CO.mutex);
    for (int i = 0; i < lme_layer_objs.lme_as_man->CO.co_n; i++) {
        ld_lock(&mac_layer_objs.COS.mutex);
        if (judge_co(lme_layer_objs.lme_as_man->CO.co[i])) {
            uint16_t ind = lme_layer_objs.lme_as_man->CO.co[i] - mac_layer_objs.COS.value;
            sdus->blks[ind] = init_buffer_ptr(DC_BLK_LEN_MAX);
            buffer_t *buf = NULL;

            if (pqueue_empty(mac_layer_objs.dc_pq)) {
                //TODO： 临时的，在攻击端引入信号强度之后需要恢复
                /* 9.7.4
                 * If no control messages are being requested for transmission a keep-alive control message shall be generated by the MAC.
                 * 生成保活消息
                 */
                // buf = gen_pdu(&(dc_keep_alive_t){.d_type = DC_TYP_KEEP_ALIVE},
                //               dc_format_descs[DC_TYP_KEEP_ALIVE].f_desc, "DC KEEP ALIVE");
                // cat_to_buffer(sdus->blks[ind], buf->ptr, buf->len);
                // free_buffer(buf);
                ld_unlock(&mac_layer_objs.COS.mutex);
                free_sdu_s(sdus);
                break;
            } else {
                mac_layer_objs.dc_curr_sz = 0;
                while (!pqueue_empty(mac_layer_objs.dc_pq) && mac_layer_objs.dc_curr_sz <= DC_SDU_LEN_MAX) {
                    pqueue_pop(mac_layer_objs.dc_pq, (void **) &node);
                    if (node == NULL) {
                        ld_unlock(&mac_layer_objs.COS.mutex);
                        ld_unlock(&lme_layer_objs.lme_as_man->CO.mutex);
                        free_sdu_s(sdus);
                        return LD_ERR_NULL;
                    }
                    buf = node->n_data;
                    cat_to_buffer(sdus->blks[ind], buf->ptr, buf->len);
                    mac_layer_objs.dc_curr_sz += buf->len;
                    free_queue_node(node);
                    node = NULL;
                    buf = NULL;
                }
            }

            /* The DC slot length is 83-bit, the last 8bits are CRC. Because 83 bits are not byte aligned,
             * each DC slot can be aligned to a maximum of 72 bits, which is 9 bytes.
             * Therefore, for an 83 bit (11 bytes) message, a CRC of 0-8 bytes needs to be calculated,
             * and the result is placed in the fourth bit of 9 bytes (75 bit) to the third bit of 10 bytes (83 bit) */
            uint8_t dc_crc = cal_crc_8bits(sdus->blks[ind]->ptr, sdus->blks[ind]->ptr + DC_BLK_LEN_MAX - 2);
            sdus->blks[ind]->ptr[9] = (dc_crc >> 3) & 0x1F;
            sdus->blks[ind]->ptr[10] = (dc_crc << 5) & 0xE0;

            change_buffer_len(sdus->blks[ind], sdus->blks[ind]->total);

            preempt_prim(&PHY_DC_REQ_PRIM, E_TYP_ANY, sdus, free_sdu_s, 0, 0);
        }
        ld_unlock(&mac_layer_objs.COS.mutex);
    }
    ld_unlock(&lme_layer_objs.lme_as_man->CO.mutex);
    return LD_OK;
}


l_err generate_data_pkt() {
    if (config.role == LD_GS) {
        if (mac_layer_objs.cd_to_trans_node == NULL) {
            log_error("CC-DCH To Trans Node is NULL");
            return LD_ERR_NULL;
        }
        ld_lock(&mac_layer_objs.CCL.mutex);
        size_t blk0_sz = FL_DATA_BLK_3_F_LEN_MAX - ((mac_layer_objs.CCL.value - 1) * CC_BLK_LEN_MIN);
        ld_unlock(&mac_layer_objs.CCL.mutex);

        buffer_t *sdu_buf = init_buffer_ptr(blk0_sz + FL_DATA_BLK_2_F_LEN_MAX * 3);

        dls_data_req_t *dls_data_req = NULL;
        while (stop_flag == FALSE) {
            lfqueue_get(mac_layer_objs.cd_to_trans_node->dch_q, (void **) &dls_data_req);
            if (dls_data_req == NULL) break;
            cat_to_buffer(sdu_buf, dls_data_req->mac_sdu->ptr, dls_data_req->mac_sdu->len);

            // log_buf(LOG_FATAL, "FL BUF", dls_data_req->mac_sdu->ptr, dls_data_req->mac_sdu->len);
            free_dls_data_req(dls_data_req);
        }

        free_cc_dch_trans_node(mac_layer_objs.cd_to_trans_node);
        mac_layer_objs.cd_to_trans_node = NULL;


        sdu_s_t *sdus = create_sdu_s(FL_DATA_BLK_N);
        sdus->blks[0] = init_buffer_unptr();
        sdus->blks[1] = init_buffer_unptr();
        sdus->blks[2] = init_buffer_unptr();
        sdus->blks[3] = init_buffer_unptr();

        CLONE_TO_CHUNK(*sdus->blks[0], sdu_buf->ptr, blk0_sz);
        CLONE_TO_CHUNK(*sdus->blks[1], sdu_buf->ptr + blk0_sz, FL_DATA_BLK_2_F_LEN_MAX);
        CLONE_TO_CHUNK(*sdus->blks[2], sdu_buf->ptr + blk0_sz + FL_DATA_BLK_2_F_LEN_MAX, FL_DATA_BLK_2_F_LEN_MAX);
        CLONE_TO_CHUNK(*sdus->blks[3], sdu_buf->ptr + blk0_sz + FL_DATA_BLK_2_F_LEN_MAX * 2, FL_DATA_BLK_2_F_LEN_MAX);

        free_buffer(sdu_buf);
        preempt_prim(&PHY_DATA_REQ_PRIM, E_TYP_FL, sdus, free_sdu_s, 0, 0);
    } else if (config.role == LD_AS) {
        ld_lock(&mac_layer_objs.COL.mutex);
        ld_lock(&mac_layer_objs.RPSO.mutex);
        ld_lock(&mac_layer_objs.NRPS.mutex);

        uint16_t dch_blks = RL_SLOT_MAX;
        sdu_s_t *sdus = create_sdu_s(dch_blks);

        /*TODO：当test_dch长度不为14的倍数的时候，截断的最后一个字符串会出现不确定内容，后期需要注意 */
        uint8_t *dch_out_str = NULL;
        int dch_to_append;

        dls_data_req_t *dls_data_req = NULL;
        lfqueue_get(mac_layer_objs.rl_data_q, (void **) &dls_data_req);

        /* the offset in RL is unused */
        if (dls_data_req == NULL) {
            dch_to_append = 0;
        } else {
            dch_out_str = dls_data_req->mac_sdu->ptr;
            dch_to_append = dls_data_req->mac_sdu->len;
        }

        //TODO： 临时的，在攻击端引入信号强度之后需要删除
        if (dls_data_req == NULL || dls_data_req->mac_sdu->len == 0) {
            free_dls_data_req(dls_data_req);

            ld_unlock(&mac_layer_objs.NRPS.mutex);
            ld_unlock(&mac_layer_objs.RPSO.mutex);
            ld_unlock(&mac_layer_objs.COL.mutex);
            set_rl_param(0, 0);
            free_sdu_s(sdus);
            return LD_ERR_NULL;
        }

        for (int i = 0; i < mac_layer_objs.NRPS.value; i++) {
            uint64_t ind = i + mac_layer_objs.RPSO.value;
            if (dch_to_append > 0 && dch_out_str != NULL) {
                sdus->blks[ind] = init_buffer_unptr();
                CLONE_TO_CHUNK_L(*sdus->blks[ind], dch_out_str + i * RL_DATA_BLK_LEN_MAX,
                                 (dch_to_append < RL_DATA_BLK_LEN_MAX) ? dch_to_append : RL_DATA_BLK_LEN_MAX,
                                 RL_DATA_BLK_LEN_MAX);
                dch_to_append -= RL_DATA_BLK_LEN_MAX;
            } else {
                sdus->blks[ind] = init_buffer_ptr(RL_DATA_BLK_LEN_MAX);
            }
        }

        free_dls_data_req(dls_data_req);


        ld_unlock(&mac_layer_objs.NRPS.mutex);
        ld_unlock(&mac_layer_objs.RPSO.mutex);
        ld_unlock(&mac_layer_objs.COL.mutex);

        set_rl_param(0, 0);
        // log_info("===========RL DATA OUT===========");
        preempt_prim(&PHY_DATA_REQ_PRIM, E_TYP_RL, sdus, free_sdu_s, 0, 0);
    }
    return LD_OK;
}

l_err generate_pkt(void *args) {
    p_rtx_obj_t *rtx_obj = args;
    switch (rtx_obj->rtx_type) {
        case PHY_RTX_BC_IND:
            return generate_bc_pkt();
        case PHY_RTX_CC_IND:
            return generate_cc_pkt();
        case PHY_RTX_RA_IND:
            return generate_ra_pkt();
        case PHY_RTX_DC_IND:
            return generate_dc_pkt();
        case PHY_RTX_DATA_IND:
            return generate_data_pkt();
        default:
            return LD_ERR_INVALID;
    }
}

void P_SAPT_cb(ld_prim_t *prim) {
    /* 当角色为AS并且在FSCAN或CSCAN状态时，直接返回 */
    if (config.role == LD_AS &&
        (in_state(&mac_layer_objs.mac_fsm, mac_fsm_states[MAC_FSCANNING]) || in_state(
             &mac_layer_objs.mac_fsm, mac_fsm_states[MAC_CSCANNING]))) {
        return;
    }
    p_rtx_obj_t *rtx_obj = dup_prim_data(prim->prim_objs, sizeof(p_rtx_obj_t));
    prim->prim_err = generate_pkt(rtx_obj);
    free(rtx_obj);
}

void P_SAPD_cb(ld_prim_t *prim) {
    sdu_s_t *sdus = prim->prim_objs;
    switch (prim->prim_seq) {
        case PHY_BC_IND:
            //only BC slot 2 need to be accepted by AS
            for (int i = 1; i < 2; i++) {
                pb_stream pbs;
                init_pbs(&pbs, sdus->blks[i]->ptr, sdus->blks[i]->len, "BC");
                uint8_t B_TYP;
                lfqueue_t *queue = lfqueue_init();

                //暂存所有数据，等待MAC计算，若MAC校验通过，再把除MAC以外所有数据放入BC QUEUE中
                while ((B_TYP = ((*pbs.cur >> (BITS_PER_BYTE - B_TYPE_LEN)) & (0xFF >> (BITS_PER_BYTE - B_TYPE_LEN))))
                       != 0) {
                    /* TODO: 把MAC中的bc_data换成普通的buffer，在最后free掉 */
                    if (B_TYP == B_TYP_BC_MAC) {
                        buffer_t *mac_buf = init_buffer_unptr();
                        CLONE_TO_CHUNK(*mac_buf, pbs.cur,
                                       ((B_TYPE_LEN + CRC_8_SIZE + 4) >> 3) + get_sec_maclen(SEC_MACLEN_64));

                        bc_mac_bd_t bc_mac;
                        bc_mac.mac = init_buffer_ptr(get_sec_maclen(SEC_MACLEN_64));
                        buffer_t *to_calc_mac = init_buffer_unptr();
                        CLONE_TO_CHUNK(*to_calc_mac, pbs.start, pbs_offset(&pbs));

                        pb_stream mac_pbs;
                        init_pbs(&mac_pbs, mac_buf->ptr, mac_buf->len, "BC_MAC");
                        if (in_struct(&bc_mac, bc_format_descs[B_TYP].f_desc, &mac_pbs, NULL) == FALSE
                            || !verify_hmac_buffer(mac_layer_objs.sm3_key, bc_mac.mac, to_calc_mac,
                                                   get_sec_maclen(SEC_MACLEN_64))
                        ) {
                            lfqueue_destroy(queue);
                            lfqueue_free(queue);
                            return;
                        }
                        free_buffer(to_calc_mac);
                        free_buffer(mac_buf);
                        free_buffer(bc_mac.mac);
                        break;
                    }
                    channel_data_t *bc_data = init_channel_data(BC_CHANNEL, B_TYP, UNBIND_SAC);
                    CLONE_TO_CHUNK(*bc_data->buf, pbs.cur, bc_format_descs[B_TYP].desc_size);
                    ld_queue_node_t *q_node = init_queue_node(B_TYP, bc_data, free_channel_data);
                    lfqueue_put(queue, q_node);

                    pbs.cur += bc_format_descs[B_TYP].desc_size;
                }

                size_t queue_size = lfqueue_size(queue);
                for (int j = 0; j < queue_size; j++) {
                    ld_queue_node_t *q_node = NULL;
                    lfqueue_get(queue, (void **) &q_node);
                    lfqueue_put(mac_layer_objs.bc_q, q_node);
                }

                //TODO: 是否存在内存泄漏？
                lfqueue_destroy(queue);
                lfqueue_free(queue);
            }
            break;
        case PHY_CC_IND: {
            pb_stream pbs;
            init_pbs(&pbs, sdus->blks[0]->ptr, sdus->blks[0]->len, "CC");
            uint8_t C_TYP;
            lfqueue_t *queue = lfqueue_init();


            //读取SLOT_DESC
            {
                cc_slot_desc_t cc_sd;

                buffer_t *sd_buf = init_buffer_unptr();
                CLONE_TO_CHUNK(*sd_buf, pbs.cur, cc_format_descs[C_TYP_SLOT_DESC].desc_size);

                pb_stream sd_pbs;
                init_pbs(&sd_pbs, sd_buf->ptr, sd_buf->len, "CC_SLOT_DESC");
                if (in_struct(&cc_sd, cc_format_descs[C_TYP_SLOT_DESC].f_desc, &sd_pbs, NULL) == FALSE) {
                    free_buffer(sd_buf);
                    goto CC_INPUT_DESTORY;
                }

                ld_lock(&mac_layer_objs.CCL.mutex);
                mac_layer_objs.CCL.value = cc_sd.CCL;
                ld_unlock(&mac_layer_objs.CCL.mutex);

                mac_layer_objs.DCL = cc_sd.DCL;

                pbs.cur += cc_format_descs[C_TYP_SLOT_DESC].desc_size;

                free_buffer(sd_buf);
            }

            //暂存所有数据，等待MAC计算，若MAC校验通过，再把除MAC以外所有数据放入BC QUEUE中
            while ((C_TYP = ((*pbs.cur >> (BITS_PER_BYTE - C_TYPE_LEN)) & (0xFF >> (BITS_PER_BYTE - C_TYPE_LEN)))) !=
                   0) {
                if (C_TYP > C_TYP_CC_MAC) {
                    log_warn("Wrong CC TYPE ID");
                    goto CC_INPUT_DESTORY;
                }
                if (C_TYP == C_TYP_DCCH_DESC) {
                    cc_dcch_desc_t cc_dd;
                    buffer_t *dd_buf = init_buffer_unptr();
                    CLONE_TO_CHUNK(*dd_buf, pbs.cur,
                                   cc_format_descs[C_TYP_DCCH_DESC].desc_size);

                    pb_stream dd_pbs;
                    init_pbs(&dd_pbs, dd_buf->ptr, dd_buf->len, "CC_DCCH_DESC");
                    if (in_struct(&cc_dd, cc_format_descs[C_TYP_DCCH_DESC].f_desc, &dd_pbs, NULL) == FALSE) {
                        free_buffer(dd_buf);
                        goto CC_INPUT_DESTORY;
                    }

                    ld_lock(&mac_layer_objs.COS.mutex);
                    ld_lock(&mac_layer_objs.COM.mutex);
                    ld_lock(&mac_layer_objs.COL.mutex);

                    mac_layer_objs.COS.value = cc_dd.COS;
                    mac_layer_objs.COL.value = cc_dd.COL;
                    mac_layer_objs.COM.value = cc_dd.COM;

                    ld_unlock(&mac_layer_objs.COS.mutex);
                    ld_unlock(&mac_layer_objs.COM.mutex);
                    ld_unlock(&mac_layer_objs.COL.mutex);

                    free_buffer(dd_buf);
                }
                if (C_TYP == C_TYP_CC_MAC) {
                    buffer_t *mac_buf = init_buffer_unptr();
                    CLONE_TO_CHUNK(*mac_buf, pbs.cur,
                                   ((C_TYPE_LEN + CRC_8_SIZE + 3) >> 3) + get_sec_maclen(SEC_MACLEN_64));

                    cc_mac_bd_t cc_mac;
                    cc_mac.mac = init_buffer_ptr(get_sec_maclen(SEC_MACLEN_64));
                    buffer_t *to_calc_mac = init_buffer_unptr();
                    CLONE_TO_CHUNK(*to_calc_mac, pbs.start, pbs_offset(&pbs));

                    pb_stream mac_pbs;
                    init_pbs(&mac_pbs, mac_buf->ptr, mac_buf->len, "CC_MAC");
                    if (in_struct(&cc_mac, cc_format_descs[C_TYP].f_desc, &mac_pbs, NULL) == FALSE) {
                        goto CC_INPUT_DESTORY;
                    }

                    //计算MAC
                    if (!verify_hmac_buffer(mac_layer_objs.sm3_key, cc_mac.mac, to_calc_mac,
                                            get_sec_maclen(SEC_MACLEN_64))) {
                        goto CC_INPUT_DESTORY;
                    }
                    free_buffer(to_calc_mac);
                    free_buffer(mac_buf);
                    free_buffer(cc_mac.mac);
                    goto CC_UPLOAD;
                }
                channel_data_t *cc_data = init_channel_data(CC_CHANNEL, C_TYP, UNBIND_SAC);
                CLONE_TO_CHUNK(*cc_data->buf, pbs.cur, cc_format_descs[C_TYP].desc_size);

                ld_queue_node_t *q_node = init_pqueue_node(cc_format_descs[C_TYP].pri, C_TYP, cc_data,
                                                           free_channel_data);
                lfqueue_put(queue, q_node);

                pbs.cur += cc_format_descs[C_TYP].desc_size;
            }
        CC_UPLOAD: {
                // log_error("CC RECEIVED");
                size_t queue_size = lfqueue_size(queue);
                for (int j = 0; j < queue_size; j++) {
                    ld_queue_node_t *q_node = NULL;
                    lfqueue_get(queue, (void **) &q_node);
                    pqueue_insert(mac_layer_objs.cc_pq, q_node);
                }
            }

        CC_INPUT_DESTORY:
            //TODO: 是否存在内存泄漏？
            lfqueue_destroy(queue);
            lfqueue_free(queue);
            break;
        }
        case PHY_RA_IND: {
            // for (int i = 0; i < RA_BLK_N; i++) {
            for (int i = 0; i < 1; i++) {
                pb_stream pbs;
                init_pbs(&pbs, sdus->blks[i]->ptr, sdus->blks[i]->len, "RA");
                enum ELE_TYP R_TYP = (*pbs.cur >> (BITS_PER_BYTE - R_TYPE_LEN)) & (
                                         0xFF >> (BITS_PER_BYTE - R_TYPE_LEN));

                channel_data_t *ra_data = init_channel_data(RA_CHANNEL, R_TYP, UNBIND_SAC);

                CLONE_TO_CHUNK(*ra_data->buf, pbs.cur, ra_format_descs[R_TYP].desc_size);

                ld_queue_node_t *q_node = init_queue_node(R_TYP, ra_data, free_channel_data);
                lfqueue_put(mac_layer_objs.ra_q, q_node);
            }
            break;
        }
        case PHY_DC_IND: {
            uint8_t D_TYP;
            if (lfqueue_size(mac_layer_objs.dc_coqueue) == 0) break;
            cc_dcch_desc_t *cc_dd = NULL;
            lfqueue_get(mac_layer_objs.dc_coqueue, (void **) &cc_dd);
            if (cc_dd == NULL) break;
            // log_debug("JUDGEING CO %d %d %d %d", cc_dd->COS, cc_dd->COM, cc_dd->COL,
            //           mac_layer_objs.mac_co_sac[cc_dd->COS]);


            for (int ind = 0; ind < sdus->blk_n; ind++) {
                if (sdus->blks[ind] == NULL) continue;
                pb_stream pbs;
                init_pbs(&pbs, sdus->blks[ind]->ptr, sdus->blks[ind]->len, "DC");
                uint8_t carried_crc = ((sdus->blks[ind]->ptr[9] << 3) & 0xF8) + (
                                          (sdus->blks[ind]->ptr[10] >> 5) & 0x07);
                uint8_t calc_crc = cal_crc_8bits(sdus->blks[ind]->ptr, sdus->blks[ind]->ptr + DC_BLK_LEN_MAX - 2);
                if (carried_crc != calc_crc) {
                    log_warn("DCCH `%d` slot CRC verification failed");
                    break;
                }
                while ((D_TYP = ((*pbs.cur >> (BITS_PER_BYTE - D_TYPE_LEN)) & (0xFF >> (BITS_PER_BYTE - D_TYPE_LEN))))
                       != 0) {
                    channel_data_t *dc_data = init_channel_data(DC_CHANNEL, D_TYP,
                                                                mac_layer_objs.mac_co_sac[
                                                                    (cc_dd->COS + ind) % cc_dd->COM]);
                    CLONE_TO_CHUNK(*dc_data->buf, pbs.cur, dc_format_descs[D_TYP].desc_size);

                    ld_queue_node_t *q_node = init_pqueue_node(dc_format_descs[D_TYP].pri, D_TYP, dc_data,
                                                               free_channel_data);
                    pqueue_insert(mac_layer_objs.dc_pq, q_node);
                    pbs.cur += dc_format_descs[D_TYP].desc_size;
                }
            }
            free(cc_dd);
            break;
        }
        case PHY_DATA_IND:
            if (prim->prim_obj_typ == E_TYP_RL) {
                if (lfqueue_size(mac_layer_objs.rpso_queue) == 0) {
                    break;
                }
                ld_rpso_t *rpso_sac = NULL;
                lfqueue_get(mac_layer_objs.rpso_queue, (void **) &rpso_sac);

                for (int i = 0, n = 1; i < rpso_sac->avail_start; i += n) {
                    //TODO: avail_start长度对不上
                    if (sdus->blks[i] != NULL) {
                        channel_data_t *rl_data = init_channel_data(RL_CHANNEL, D_TYP_RL,
                                                                    rpso_sac->rpsos[i].SAC);

                        for (n = i; n < i + rpso_sac->rpsos[i].NRPS; n++) {
                            if (sdus->blks[n] != NULL) {
                                cat_to_buffer(rl_data->buf, sdus->blks[n]->ptr, sdus->blks[n]->len);
                            }
                        }

                        ld_queue_node_t *q_node = init_queue_node(D_TYP_RL, rl_data, free_channel_data);
                        lfqueue_put(mac_layer_objs.rl_data_q, q_node);
                    } else {
                        n = 1; /* step is 1 */
                    }
                }
                free(rpso_sac);
            } else {
                fl_alloc_record_t *record = NULL;
                buffer_t *input_buf = init_buffer_unptr();

                for (int i = 0; i < sdus->blk_n; i++) {
                    if (sdus->blks[i] != NULL) {
                        cat_to_buffer(input_buf, sdus->blks[i]->ptr, sdus->blks[i]->len);
                    }
                }

                while (lfqueue_get(mac_layer_objs.fl_alloc_queue, (void **) &record) == 0) {
                    if (record == NULL) continue;
                    channel_data_t *fl_data = init_channel_data(FL_CHANNEL, D_TYP_FL, UNBIND_SAC);
                    CLONE_TO_CHUNK(*fl_data->buf, input_buf->ptr + record->BO, record->BL);

                    ld_queue_node_t *q_node = init_queue_node(D_TYP_FL, fl_data, free_channel_data);
                    lfqueue_put(mac_layer_objs.fl_data_q, q_node);

                    free(record);
                }

                free_buffer(input_buf);
            }
            break;
        case PHY_BC_REQ:
            break;
        default:
            break;
    }
}

void P_SAPC_cb(ld_prim_t *prim) {
}


l_err entry_MAC_HO2(void *args) {
    l_err err = LD_OK;
    // if ((err = preempt_prim(&PHY_SYNC_REQ_PRIM)))
    // TODO: 未来这里要指定频率作为Obj
    if ((err = preempt_prim(&PHY_CONF_REQ_PRIM, PHY_TYP_HO, NULL, NULL, 0, 0)) != LD_OK) {
        log_error("Set PHY layer for new GS has failed");
        return err;
    }

    // if ((err = preempt_prim(&PHY_SYNC_REQ_PRIM, E_TYP_ANY, NULL, NULL, 0, 0)) != LD_OK) {
    //     log_error("Cannot exec PHY SYNC primitive");
    //     return err;
    // }
    return err;
}
