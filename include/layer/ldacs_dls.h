//
// Created by 邹嘉旭 on 2023/12/14.
//

#ifndef LDACS_SIM_LDACS_DLS_H
#define LDACS_SIM_LDACS_DLS_H
#include "layer_core.h"
#include "ldacs_lme.h"
#include "ldacs_snp.h"
#include  "ldacs_utils.h"

#define MAX_DLS_SDU_LEN 2048

#define DLS_SEQ_MAX 1 << 5
#define DLS_WINDOW_SIZE  1 << 5

typedef enum {
    DLS_DATA_REQ = 0x1,
    DLS_DATA_IND = 0x2,
    DLS_UDATA_REQ = 0x3,
    DLS_UDATA_IND = 0x4,
    DLS_OPEN_REQ = 0x5,
    DLS_CLOSE_REQ = 0x6,
} ld_dls_prims_en;

typedef enum {
    ACK_DATA = 0x0,
    UACK_DATA = 0x1,
} DLS_TYP;


typedef struct dls_entity_s {
    uint16_t AS_SAC;
    uint16_t GS_SAC;
    lfqueue_t *cos_iqueue[8],
            *cos_oqueue[8],
            *reassy_queue;

    window_t *iwindow,
            *owindow;

    size_t cos_req_res[8];
    pthread_mutex_t cos_req_mutex[8];
    ld_gtimer_t req_timer;

    ld_bitset_t *ack_bitset;

    pthread_t trans_wait_th,
            recv_th,
            upload_th;
    ldacs_roles role;
} dls_entity_t;

typedef struct dls_layer_objs_s {
    size_t DLS_P_SDU;
    size_t DLS_P_PDU;

    dls_entity_t *AS_DLS;
    struct hashmap *as_dls_map;
    sm_statemachine_t dls_fsm;
} dls_layer_objs_t;

extern dls_layer_objs_t dls_layer_objs;

typedef struct dls_data_req_s {
    // uuid_t uuid;
    uint16_t offset;
    buffer_t *mac_sdu;
} dls_data_req_t;

static dls_data_req_t *create_dls_data_req(uint16_t offset, buffer_t *sdu) {
    dls_data_req_t *data_req = malloc(sizeof(dls_data_req_t));
    // memcpy(data_req->uuid, uuid, 16);
    data_req->offset = offset;
    data_req->mac_sdu = sdu;
    return data_req;
}

static void free_dls_data_req(void *req) {
    dls_data_req_t *dls_req = req;
    if (dls_req) {
        if (dls_req->mac_sdu) {
            free_buffer(dls_req->mac_sdu);
        }
        free(dls_req);
    }
}


typedef struct dls_en_data_s {
    uint16_t GS_SAC;
    uint32_t AS_UA;
    uint16_t AS_SAC;
} dls_en_data_t;

enum dls_fsm_event_type {
    DLS_EV_DEFAULT = 0,
};

enum DLS_FSM_STATES_E {
    DLS_CLOSED,
    DLS_OPEN,
};

static const char *dls_fsm_states[] = {
    "DLS_CLOSED",
    "DLS_OPEN",
};

extern fsm_event_t dls_fsm_events[];

//
// typedef struct res_alloc_s {
//     size_t alloc_size;
// } res_alloc_t;

#pragma pack(1)

typedef struct dls_data_s {
    uint8_t TYP;
    uint8_t RST;
    uint8_t LFR;
    uint8_t SC;
    uint8_t PID;
    uint16_t SEQ2; // equals to SEQ1 - 1
    uint16_t LEN;
    buffer_t *DATA;
} dls_data_t;


typedef struct cc_ack_s {
    uint8_t c_type;
    uint16_t AS_SAC;
    // uint8_t SC;
    uint8_t PID;
    uint16_t bitmap;
} cc_ack_t;

typedef struct dc_ack_s {
    uint8_t d_type;
    // uint8_t SC;
    uint8_t PID;
    uint16_t bitmap;
} dc_ack_t;

typedef struct cc_frag_ack_s {
    uint8_t c_type;
    uint16_t AS_SAC;
    // uint8_t SC;
    uint8_t PID;
    uint16_t SEQ1;
} cc_frag_ack_t;

typedef struct dc_frag_ack_s {
    uint8_t d_type;
    // uint8_t SC;
    uint8_t PID;
    uint16_t SEQ1;
} dc_frag_ack_t;

#pragma pack()
#define DATA_TYPE_LEN 1
#define DATA_HEAD_LEN 9  /* (1b+1b+1b+3b+5b+11b+11b+7b+32b) / 8 = 9B */

extern ld_prim_t DLS_DATA_REQ_PRIM;
extern ld_prim_t DLS_DATA_IND_PRIM;
extern ld_prim_t DLS_UDATA_REQ_PRIM;
extern ld_prim_t DLS_UDATA_IND_PRIM;
extern ld_prim_t DLS_OPEN_REQ_PRIM;
extern ld_prim_t DLS_CLOSE_REQ_PRIM;

static const size_t dls_data_crc_size = CRC_32_SIZE;

extern struct_desc_t dls_data_desc;
extern struct_desc_t cc_ack_desc;
extern struct_desc_t dc_ack_desc;
extern struct_desc_t cc_frag_ack_desc;
extern struct_desc_t dc_frag_ack_desc;

l_err make_dls_layer();

void D_SAPD(ld_prim_t *prim);

void D_SAPC(ld_prim_t *prim);

void M_SAPD_cb(ld_prim_t *prim);

void M_SAPC_D_cb(ld_prim_t *prim);

void L_SAPR_cb(ld_prim_t *prim);

/* dls_entity.c */
dls_entity_t *init_dls_entity(uint16_t src_sac, uint16_t dest_sac, ldacs_roles role);

l_err clear_dls_entity(dls_entity_t *en);

//dls_entity_t *init_dls_as_entity(uint16_t gs_sac, uint16_t as_sac, uint32_t as_ua, uint8_t SCGS, uint8_t ver);
dls_entity_t *init_dls_as_entity(uint16_t gs_sac, uint16_t as_sac);


static bool dls_enode_iter(const void *item, void *udata) {
    const dls_entity_t *user = item;
    return TRUE;
}


l_err recv_ack(dls_entity_t *en, uint8_t PID, uint16_t bitmap);

l_err recv_frag_ack(dls_entity_t *en, uint8_t PID, uint16_t SEQ1);

l_err dls_frag_func(dls_entity_t *en, size_t alloced);

/* util */

void init_dls_fsm(dls_layer_objs_t *dls_obj, enum DLS_FSM_STATES_E init_state);

static uint64_t dls_enode_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const dls_entity_t *node = item;
    return hashmap_sip(&node->AS_SAC, sizeof(uint16_t), seed0, seed1);
}

static dls_entity_t *get_dls_enode(const uint16_t as_sac) {
    return hashmap_get(dls_layer_objs.as_dls_map, &(dls_entity_t){.AS_SAC = as_sac});
}

static bool has_dls_enode(const uint16_t as_sac) {
    return hashmap_get(dls_layer_objs.as_dls_map, &(dls_entity_t){.AS_SAC = as_sac}) != NULL;
}

static const void *set_dls_enode(uint16_t gs_sac, uint16_t as_sac) {
    dls_entity_t *en = init_dls_as_entity(gs_sac, as_sac);
    const void *ret = NULL;
    if (en != NULL) {
        ret = hashmap_set(dls_layer_objs.as_dls_map, en);
        // free(en);
    }
    return ret;
}


static struct hashmap *dls_init_map() {
    return hashmap_new(sizeof(dls_entity_t), 0, 0, 0,
                       dls_enode_hash, NULL, NULL, NULL);
}

static const void *dls_delete_map_node(const uint16_t SAC, l_err (*clear_func)(dls_entity_t *)) {
    dls_entity_t *en = get_dls_enode(SAC);
    if (en) {
        clear_func(en);
        return hashmap_delete(dls_layer_objs.as_dls_map, en);
    }
    return NULL;
}

// static void dls_delete_node_by_ua(uint16_t ua_as) {
//     const dls_entity_t *node = NULL;
//     if ((node = get_dls_enode(ua_as))) {
//         dls_delete_map_node(node);
//     }
// }


void delete_dls_enode_by_ua(uint16_t ua_as);
#endif //LDACS_SIM_LDACS_DLS_H
