//
// Created by jiaxv on 2023/12/5.
//

#ifndef LDACS_SIM_LDACS_MAC_H
#define LDACS_SIM_LDACS_MAC_H

#include "layer_core.h"
#include "ldacs_lme.h"
#include "ldacs_dls.h"
#include "ldacs_phy.h"

#define C_TYPE_NUMBER 32
#define D_TYPE_NUMBER 16
#define R_TYPE_NUMBER 4

#define RACH_PER_SF 2


enum MAC_FSM_STATES_E {
    MAC_FSCANNING = 0,
    MAC_CSCANNING,
    MAC_CONNECTING,
    MAC_AUTH,
    MAC_OPEN,
    MAC_HO2,
};

enum mac_fsm_event_type {
    MAC_EV_DEFAULT = 0,
};

extern const char *mac_fsm_states[];
extern fsm_event_t mac_fsm_events[];


typedef struct ld_rpso_map_s {
    uint16_t SAC;
    uint8_t NRPS;
} ld_rpso_map_t;

typedef struct ld_rpso_s {
    ld_rpso_map_t rpsos[RL_SLOT_MAX];
    uint8_t avail_start;
    size_t avail_sz;
} ld_rpso_t;

typedef struct input_struct_s {
    void *queue_p;

    void (*process_func)(const ld_queue_node_t *);

    void *(*wait_while)(void *);

    int role;

    pthread_t th;
} input_struct_t;

typedef struct fl_alloc_record_s {
    uint16_t BO;
    uint16_t BL;
} fl_alloc_record_t;

typedef void (*inter_func)(ld_queue_node_t *);

typedef struct mac_interv_s {
    inter_func func;
    struct list_head lpointer; //链表节点
} mac_interv_t;

static mac_interv_t *init_mac_interv(inter_func func) {
    mac_interv_t *interv = malloc(sizeof(mac_interv_t));
    interv->func = func;
    return interv;
}

typedef struct cc_dch_trans_node_s {
    pqueue_t *cc_out_pq;
    lfqueue_t *dch_q;
} cc_dch_trans_node_t;

static cc_dch_trans_node_t *init_cc_dch_trans_node() {
    cc_dch_trans_node_t *node = calloc(1, sizeof(cc_dch_trans_node_t));
    node->cc_out_pq = pqueue_init(MAX_HEAP, ld_cmp_pri, ld_get_pri, ld_set_pri, ld_get_pos,
                                  ld_set_pos);
    node->dch_q = lfqueue_init();
    return node;
}

static void free_cc_dch_trans_node(cc_dch_trans_node_t *node) {
    if (node) {
        if (node->cc_out_pq) {
            pqueue_free(node->cc_out_pq);
        }
        if (node->dch_q) {
            lfqueue_destroy(node->dch_q);
            lfqueue_free(node->dch_q);
        }
    }
    free(node);
}

typedef struct mac_layer_objs_s {
    enum MAC_FSM_STATES_E state;
    uint32_t mac_c_rac;
    // enum p_sec mac_p_sec;

    bool isAPNT;
    size_t hmac_len;
    KEY_HANDLE sm3_key;

    // queue_t *bc_q; /* the basic queue of bcch */
    lfqueue_t *bc_q; /* the basic queue of bcch */
    pqueue_t *cc_pq;
    pqueue_t *dc_pq;
    lfqueue_t *ra_q;
    lfqueue_t *fl_data_q;
    lfqueue_t *rl_data_q;

    // pqueue_t *cc_curr_out_q;
    cc_dch_trans_node_t *cd_asseming_node;
    cc_dch_trans_node_t *cd_to_trans_node;
    lfqueue_t *cd_assemed_qs;
    pthread_mutex_t cd_assq_mutex;
    size_t dc_curr_sz;

    lfqueue_t *bc_curr_out_q;
    lfqueue_t *bc_assem_qs;
    pthread_mutex_t bc_assq_mutex;

    input_struct_t bc_input,
            cc_input,
            dc_input,
            ra_input,
            fl_data_input,
            rl_data_input;

    pthread_t rtx_bc_th;
    pthread_t rpso_th;

    uint8_t cc_order[C_TYPE_NUMBER];
    uint8_t dc_priority[D_TYPE_NUMBER];
    uint8_t ra_priority[R_TYPE_NUMBER];


    safe_bigint_t COS;
    safe_bigint_t COM;
    safe_bigint_t COL;

    safe_bigint_t RPSO;
    safe_bigint_t NRPS;

    safe_bigint_t CCL;

    /* for GS */
    uint16_t mac_co_sac[CO_MAX];
    ld_rpso_map_t mac_rpso_sac[RL_SLOT_MAX];
    lfqueue_t *dc_coqueue;
    lfqueue_t *rpso_queue;

    /* for AS */
    lfqueue_t *fl_alloc_queue;
    // uint8_t CCL;
    uint8_t DCL;

    sm_statemachine_t mac_fsm;

    /* intervene */
    mac_interv_t intervs;
    struct list_head *interv_head;
} mac_layer_objs_t;

#define UNBIND_SAC -1

typedef struct channel_data_s {
    enum CHANNEL_E channel;
    enum ELE_TYP type;
    uint16_t SAC;
    buffer_t *buf;
} channel_data_t;


extern channel_data_t *init_channel_data(enum CHANNEL_E channel, enum ELE_TYP type, uint16_t SAC);

extern void free_channel_data(void *rdata);

extern channel_data_t *dup_channel_data(channel_data_t *src);

typedef enum {
    MAC_CONNECT_REQ = 0x101,
    MAC_FSCAN_REQ = 0x102,
    MAC_CSCAN_REQ = 0x103,
    MAC_GSCAN_REQ = 0x104,
    MAC_OPEN_REQ = 0x105,
    MAC_SYNC_REQ = 0x106,
    MAC_HO_REQ = 0x107,
    MAC_AUTH_REQ = 0x108,
    MAC_SYNC_IND = 0x109,
    MAC_CC_STATUS_REQ = 0x10A,
    MAC_BCCH_REQ = 0x10B,
    MAC_BCCH_IND = 0x10C,
    MAC_RACH_REQ = 0x10D,
    MAC_RACH_IND = 0x10E,
    MAC_CCCH_REQ = 0x10F,
    MAC_CCCH_IND = 0x110,
    MAC_DCCH_REQ = 0x111,
    MAC_DCCH_IND = 0x112,
    MAC_DCH_REQ = 0x113,
    MAC_DCH_IND = 0x114,
} ld_mac_prims_en;

extern ld_prim_t MAC_CONNECT_REQ_PRIM;
extern ld_prim_t MAC_FSCAN_REQ_PRIM;
extern ld_prim_t MAC_CSCAN_REQ_PRIM;
extern ld_prim_t MAC_GSCAN_REQ_PRIM;
extern ld_prim_t MAC_OPEN_REQ_PRIM;
extern ld_prim_t MAC_SYNC_REQ_PRIM;
extern ld_prim_t MAC_HO_REQ_PRIM;
extern ld_prim_t MAC_AUTH_REQ_PRIM;
extern ld_prim_t MAC_SYNC_IND_PRIM;
extern ld_prim_t MAC_CC_STATUS_REQ_PRIM;
extern ld_prim_t MAC_BCCH_REQ_PRIM;
extern ld_prim_t MAC_BCCH_IND_PRIM;
extern ld_prim_t MAC_RACH_REQ_PRIM;
extern ld_prim_t MAC_RACH_IND_PRIM;
extern ld_prim_t MAC_CCCH_REQ_PRIM;
extern ld_prim_t MAC_CCCH_IND_PRIM;
extern ld_prim_t MAC_DCCH_REQ_PRIM;
extern ld_prim_t MAC_DCCH_IND_PRIM;
extern ld_prim_t MAC_DCH_REQ_PRIM;
extern ld_prim_t MAC_DCH_IND_PRIM;


extern lyr_desc_t mac_desc;


static void init_bc_input_queue(const ld_queue_node_t *node);

static void init_cc_input_queue(const ld_queue_node_t *node);

static void init_dc_input_queue(const ld_queue_node_t *node);

static void init_ra_input_queue(const ld_queue_node_t *node);

static void init_fl_data_input_queue(const ld_queue_node_t *node);

static void init_rl_data_input_queue(const ld_queue_node_t *node);

void init_mac_fsm(mac_layer_objs_t *mac_obj, enum MAC_FSM_STATES_E init_state);

l_err make_mac_layer();

void M_SAPI(ld_prim_t *prim);

void M_SAPB(ld_prim_t *prim);

void M_SAPR(ld_prim_t *prim);

void M_SAPC(ld_prim_t *prim);

void M_SAPD(ld_prim_t *prim);

void P_SAPD_cb(ld_prim_t *prim);

void P_SAPT_cb(ld_prim_t *prim);

void P_SAPC_cb(ld_prim_t *prim);

extern mac_layer_objs_t mac_layer_objs;

static void set_mac_CO(uint16_t CO, uint16_t SAC) {
    mac_layer_objs.mac_co_sac[CO] = SAC;
}

static l_err clear_mac_CO(uint16_t SAC, uint16_t *CO) {
    for (int i = 0; i < CO_MAX; i++) {
        if (mac_layer_objs.mac_co_sac[i] == SAC) {
            *CO = i;
            return LD_OK;
        }
    }
    return LD_ERR_WRONG_PARA;
}

//
// static void set_RPSO(uint8_t RPSO, uint8_t NRPS, uint16_t SAC) {
//     mac_layer_objs.mac_rpso_sac[RPSO].SAC = SAC;
//     mac_layer_objs.mac_rpso_sac[RPSO].NRPS = NRPS;
// }

void set_rpsos(ld_rpso_t *rpsos);

ld_rpso_t *init_rpsos(uint8_t start, size_t size);

l_err mac_register_interv(inter_func func);

l_err set_rl_param(uint8_t RPSO, uint8_t NRPS);

#endif //LDACS_SIM_LDACS_MAC_H
