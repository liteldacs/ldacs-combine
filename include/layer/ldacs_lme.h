//
// Created by 邹嘉旭 on 2023/12/14.
//

#ifndef LDACS_SIM_LDACS_LME_H
#define LDACS_SIM_LDACS_LME_H

#include "layer_core.h"
#include "layer_rcu.h"
#include "ldacs_mac.h"
#include "ldacs_dls.h"

#define TIMER_MAX 65535
#define INIT_FL_CHN     300     /* which mapping to 1110.0 MHZ */
#define INIT_RL_CHN     8       /* which mapping to 964.0 MHZ */

#define EPLDACS_LEN 128  /* 128 bits */

#define MAX_ORDER 0xFF

#define RPSO_DELAY_INTVL  MF_TIMER
#define GTYP_LEN 4

#define DEFAULT_GSNF_VERSION 0x01

#define AUTHC_ALG_S_LEN 4
#define AUTHC_KLEN_LEN 2

#define SHAREDINFO_LEN 21


enum CMS_en {
    CMS_TYP_1 = 0, /* QPSK with 1/2 code-rate */
    CMS_TYP_2 = 1, /* QPSK with 2/3 code-rate */
    CMS_TYP_3 = 2, /* QPSK with 3/4 code-rate */
    CMS_TYP_4 = 3, /* 16QAM with 1/2 code-rate */
    CMS_TYP_5 = 4, /* 16QAM with 2/3 code-rate */
    CMS_TYP_6 = 5, /* 64QAM with 1/2 code-rate */
    CMS_TYP_7 = 6, /* 64QAM with 2/3 code-rate */
    CMS_TYP_8 = 7, /* 64QAM with 3/4 code-rate */
};

#define CMS_N 8

typedef enum {
    LME_OPEN_REQ = 0x1,
    LME_CONF_REQ = 0x2,
    LME_AUTH_REQ = 0x3,
    LME_STATE_IND = 0x4,
    LME_TEST_REQ = 0x5,
    LME_R_REQ = 0x6,
    LME_R_IND = 0x7,
} ld_lme_prims_en;

enum MODULATION_en {
    QPSK = 0,
    QAM_16,
    QAM_64,
};

enum MOD_en {
    MOD_USER_SPECIFIC = 0,
    MOD_CELL_SPECIFIC = 1,
};

enum lme_timer_index {
    LME_TIMER_BC_TEST = 0,
    LME_TIMER_CC_TEST,
    LME_TIMER_RA_CR,
    //LME_RMS_RRM,
};

enum LME_FINISHED_STATE_E {
    LME_NO_STATE_FINISHED = 0,
    LME_CSCANNING_FINISHED,
    LME_CONNECTING_FINISHED,
    LME_AUTH_FINISHED,
};

enum lme_fsm_event_type {
    LME_EV_DEFAULT = 0,
};

enum LME_FSM_STATES_E {
    LME_FSCANNING,
    LME_CSCANNING,
    LME_CONNECTING,
    LME_AUTH,
    LME_OPEN,
};


enum authc_fsm_event_type {
    LD_AUTHC_EV_DEFAULT = 0,
};

enum LD_AUTHC_STATES_E {
    LD_AUTHC_A0 = 0,
    LD_AUTHC_A1,
    LD_AUTHC_A2,
    LD_AUTHC_G0,
    LD_AUTHC_G1,
    LD_AUTHC_G2,
};


enum AUTHC_MACLEN_E {
    AUTHC_MACLEN_INVALID = 0x0,
    AUTHC_MACLEN_96 = 0x1,
    AUTHC_MACLEN_128 = 0x2,
    AUTHC_MACLEN_64 = 0x3,
    AUTHC_MACLEN_256 = 0x4,
};

enum AUTHC_AUTHID_E {
    AUTHC_AUTH_INVALID = 0x0,
    AUTHC_AUTH_SM3HMAC = 0x1,
    AUTHC_AUTH_SM2_WITH_SM3 = 0x2,
};

enum AUTHC_ENC_E {
    AUTHC_ENC_INVALID = 0x0,
    AUTHC_ENC_SM4_CBC = 0x1,
    AUTHC_ENC_SM4_CFB = 0x2,
    AUTHC_ENC_SM4_OFB = 0x3,
    AUTHC_ENC_SM4_ECB = 0x4,
    AUTHC_ENC_SM4_CTR = 0x5,
};

enum AUTHC_KLEN_E {
    AUTHC_KLEN_RESERVED = 0x0,
    AUTHC_KLEN_128 = 0x1,
    AUTHC_KLEN_256 = 0x2,
};


enum PID_E {
    PID_RESERVED = 0x0,
    PID_SIGN = 0x1,
    PID_MAC = 0x2,
    PID_BOTH = 0x3,
};


extern const char *lme_fsm_states[];

extern enum_names sib_mod_names;
extern enum_names sib_cms_names;

extern fsm_event_t lme_fsm_events[];

typedef struct ACM_s {
    enum CMS_en acm_m;
    enum MODULATION_en mod;
    double cc;
    double res_cons;
    int32_t GS_rec_sens;
    int32_t AS_rec_sens;
    uint8_t FL_uncoded_bits;
    uint8_t RL_uncoded_bits;
} ACM_t;


typedef struct ld_co_s {
    pthread_mutex_t mutex;
    size_t co_n;
    uint16_t co[DC_SLOT_MAX];
} ld_co_t;

l_err add_co(ld_co_t *ld_co, uint16_t new_CO);


typedef struct lme_as_man_s {
    uint32_t AS_UA;
    uint16_t AS_SAC;
    uint16_t AS_CURR_GS_SAC; /* current connected/to connect GS SAC for AS */

//    uint8_t AUTHC_MACLEN,
//            AUTHC_AUTH_ID,
//            AUTHC_ENC_ID,
//            AUTHC_KLEN;
    ld_co_t CO;

    // for GS
    bool gs_finish_auth;

    uint8_t RPSO,
            NRPS;

    uint32_t send_T_SQN,
            recv_T_SQN;

    /* for GSC */
    uint32_t gsnf_count;
} lme_as_man_t;

lme_as_man_t *init_as_man(uint16_t AS_SAC, uint32_t AS_UA, uint16_t AS_CURR_GS_SAC);

l_err clear_as_man(lme_as_man_t *as_man);

l_err free_as_man(lme_as_man_t *as_man);

l_err as_man_update_handler(lme_as_man_t *as_man);

l_err as_man_update_key_handler(lme_as_man_t *as_man, void *key, uint64_t value, size_t sz, const char *key_str);

extern ACM_t acm[CMS_N];

typedef struct lme_layer_objs_s {
    uint32_t LME_T_CELL_RESP,
            LME_T_MAKE,
            LME_T_PRLA,
            LME_T_RLK,
            LME_T_FLK,
            LME_T_CSAN;
    uint32_t LME_T1_FLK;
    uint32_t LME_C_MAKE,
            LME_C_PRLA;
    uint32_t SF_NUMBER,
            MF_NUMBER;

    uint8_t PROTOCOL_VER;
    enum MOD_en MOD;
    enum CMS_en CMS;
    uint8_t EIRP;

    uint16_t init_flf;
    uint16_t init_rlf;

    ld_gtimer_t cc_timer;
    gtimer_ev_t gtimer[10];

    /* LME internal states */
    enum LME_FINISHED_STATE_E finish_status;

    /* */
    uint16_t GS_SAC;

    /* lme状态机 */
    sm_statemachine_t lme_fsm;

    /* AS DLS singal DLS is defined in DLS layer */
    /* GS <-> AS multi DLS is defined in LME layer */
    struct hashmap *LME_GS_AUTH_AS;

    /* ====================  for AS ==================== */
    lme_as_man_t *lme_as_man;
} lme_layer_objs_t;

extern lme_layer_objs_t lme_layer_objs;

typedef struct lme_plane_para_s {
    const uint8_t VER;
    uint16_t fl_freq;
    uint16_t rl_freq;
    uint8_t MOD;
    uint8_t CMS;
    uint8_t EIRP;
} lme_plane_para_t;

// typedef struct res_req_s {
//     size_t req_size; /* require by dls */
//     DLS_COS cos;
//     void *d_entity; /* actually dls_entity_t  */
// } res_req_t;


typedef struct lme_en_data_s {
    uint16_t GS_SAC;
    uint32_t AS_UA;
    uint16_t AS_SAC;
    uint8_t SCGS;
    uint8_t VER;
} lme_en_data_t;

// static res_req_t *create_res_req() {
//     res_req_t *res_req = malloc(sizeof(res_req_t));
//     res_req->req_size = req_size;
//     res_req->cos = cos;
//     res_req->d_entity = d_entity;
//     return res_req;
// }


#pragma pack(1)
typedef struct bc_acb_s {
    uint8_t b_type;
    uint16_t SAC;
    uint16_t FLF;
    uint16_t RLF;
} bc_acb_t;

typedef struct bc_sib_s {
    uint8_t b_type;
    uint16_t SAC;
    uint8_t VER;
    uint16_t FLF;
    uint16_t RLF;
    uint8_t MOD;
    uint8_t CMS;
    uint8_t EIRP;
} bc_sib_t;

typedef struct bc_mac_bd_s {
    uint8_t b_type;
    buffer_t *mac;
    uint16_t mac_len;
} bc_mac_bd_t;

static void free_bc_mac_bd(void *ptr) {
    bc_mac_bd_t *bd = ptr;
    if (ptr) {
        //free_buffer_v(&bd->mac);
        free_buffer(bd->mac);
        free(bd);
    }
}

typedef struct cc_slot_desc_s {
    uint8_t CCL;
    uint8_t DCL;
} cc_slot_desc_t;

typedef struct cc_dcch_desc_s {
    uint8_t c_type;
    uint8_t COL;
    uint16_t COS;
    uint16_t COM;
} cc_dcch_desc_t;

typedef struct cc_cell_resp_s {
    uint8_t c_type;
    uint16_t SAC;
    uint32_t UA;
    uint8_t PAV;
    uint16_t FAV;
    uint16_t TAV;
    uint16_t CO;
    buffer_t *EPLDACS;
    uint16_t CCLDACS;
    uint8_t VER;
} cc_cell_resp_t;

static void free_cc_cell_resp(void *args) {
    cc_cell_resp_t *resp = args;
    if (!resp->EPLDACS) {
        free_buffer(resp->EPLDACS);
    }
    free(resp);
}

typedef struct cc_fl_alloc_s {
    uint8_t c_type;
    uint16_t SAC;
    uint16_t BO;
    uint16_t BL;
} cc_fl_alloc_t;

typedef struct cc_rl_alloc_s {
    uint8_t c_type;
    uint16_t SAC;
    uint8_t RPSO;
    uint8_t NRPS;
    uint8_t CMS;
} cc_rl_alloc_t;


typedef struct cc_mac_bd_s {
    uint8_t c_type;
    buffer_t *mac;
    uint16_t mac_len;
} cc_mac_bd_t;

static void free_cc_mac_bd(void *ptr) {
    cc_mac_bd_t *bd = ptr;
    if (ptr) {
        free_buffer(bd->mac);
        free(bd);
    }
}


typedef struct cc_rsc_rqst_s {
    uint16_t SAC;
    uint8_t SC;
    uint16_t resource;
} cc_rsc_t;

typedef struct ra_cell_rqst_s {
    uint8_t r_type;
    uint32_t UA;
    uint16_t SAC; /* GS SAC */
    uint8_t SCGS;
    uint8_t VER;
} ra_cell_rqst_t;

typedef struct dc_keep_alive_s {
    uint8_t d_type;
} dc_keep_alive_t;

typedef struct dc_rsc_rqst_s {
    uint8_t d_type;
    uint8_t SC;
    uint16_t REQ;
} dc_rsc_rqst_t;


typedef struct dc_cell_exit_s {
    uint8_t d_type;
    uint16_t SAC;
} dc_cell_exit_t;


#pragma pack()

extern lyr_desc_t lme_desc;

extern struct_desc_t bc_acb_desc;
extern struct_desc_t bc_sib_desc;
extern struct_desc_t bc_hmac_desc;
extern struct_desc_t cc_slot_desc;
extern struct_desc_t cc_dcch_desc;
extern struct_desc_t cc_cell_resp_desc;
extern struct_desc_t cc_fl_alloc_desc;
extern struct_desc_t cc_rl_alloc_desc;
extern struct_desc_t cc_hmac_desc;
extern struct_desc_t ra_cell_rqst_desc;
extern struct_desc_t dc_keep_alive_desc;
extern struct_desc_t dc_rsc_rqst_desc;
extern struct_desc_t dc_cell_exit_desc;

extern ld_format_desc_t bc_format_descs[];
extern ld_format_desc_t cc_format_descs[];
extern ld_format_desc_t ra_format_descs[];
extern ld_format_desc_t dc_format_descs[];


extern ld_prim_t LME_OPEN_REQ_PRIM;
extern ld_prim_t LME_CONF_REQ_PRIM;
extern ld_prim_t LME_AUTH_REQ_PRIM;
extern ld_prim_t LME_STATE_IND_PRIM;
extern ld_prim_t LME_R_REQ_PRIM;
extern ld_prim_t LME_R_IND_PRIM;

l_err make_lme_layer();

void L_SAPC(ld_prim_t *prim);

void L_SAPR(ld_prim_t *prim);

void L_SAPT(ld_prim_t *prim);

void M_SAPI_cb(ld_prim_t *prim);

void M_SAPB_cb(ld_prim_t *prim);

void M_SAPC_L_cb(ld_prim_t *prim);

void M_SAPR_cb(ld_prim_t *prim);


void SN_SAPC_cb(ld_prim_t *prim);

void SN_SAPD_L_cb(ld_prim_t *prim);

void D_SAPC_cb(ld_prim_t *prim);

l_err change_LME_CONNECTING();

int8_t as_finish_auth_func();

l_err change_LME_OPEN();

void exit_LME_CONN_OPEN_action(void *curr_st_data, struct sm_event_s *event, void *new_state_data);

int8_t trans_snp_data(uint16_t AS_SAC, uint16_t GS_SAC, uint8_t *buf, size_t buf_len);

int8_t register_snf_failed(uint16_t SAC);

/* mms */
l_err init_lme_mms(lme_layer_objs_t *obj);

void *trans_lme_bc_timer_func(void *args);

void trans_bc_acb_func(void *args);

void trans_bc_sib_func(void *args);

void trans_bc_mac_timer_func(evutil_socket_t fd, short event, void *arg);

void *trans_ra_cr_timer_func(void *args);

l_err start_mms();

void free_CO(uint16_t co);


/* rms */
l_err init_lme_rms(lme_layer_objs_t *obj);

// l_err lme_req_register(res_req_t *res_req);

void *trans_lme_cc_timer_func(void *args);

void trans_cc_sd_timer_func(void *args);

void trans_cc_dd_func(void *args);

void trans_cc_mac_timer_func(void *args);

void rrm_timer_func(void *args);


/* lme util */

void init_lme_fsm(lme_layer_objs_t *lme_obj, enum LME_FSM_STATES_E init_state);

static uint64_t hash_lme_as_enode(const void *item, uint64_t seed0, uint64_t seed1) {
    const lme_as_man_t *node = item;
    return hashmap_sip(&node->AS_SAC, sizeof(uint16_t), seed0, seed1);
}

static lme_as_man_t *get_lme_as_enode(const uint16_t as_sac) {
    return hashmap_get(lme_layer_objs.LME_GS_AUTH_AS, &(lme_as_man_t) {.AS_SAC = as_sac});
}

static const bool has_lme_as_enode(const uint16_t as_sac) {
    return hashmap_get(lme_layer_objs.LME_GS_AUTH_AS, &(lme_as_man_t) {.AS_SAC = as_sac}) != NULL;
}

static lme_as_man_t * get_lme_as_enode_by_ua(const uint32_t UA) {
    size_t iter = 0;
    void *item;
    while (hashmap_iter(lme_layer_objs.LME_GS_AUTH_AS, &iter, &item)) {
        const lme_as_man_t *as_man = item;
        if (as_man->AS_UA == UA)
            return item;
    }
    return NULL;
}

static struct hashmap *init_lme_sac_map() {
    return hashmap_new(sizeof(lme_as_man_t), 0, 0, 0,
                       hash_lme_as_enode, NULL, NULL, NULL);
}

static const void *set_lme_as_enode(lme_as_man_t *as_man) {
    const void *ret = hashmap_set(lme_layer_objs.LME_GS_AUTH_AS, as_man);

    // free_as_man(as_man);
    free(as_man);

    return ret;
}

static const void *delete_lme_as_node_by_sac(uint16_t as_sac, l_err (*clear_func)(lme_as_man_t *as_man)) {
    lme_as_man_t *as_man = get_lme_as_enode(as_sac);
    if (as_man) {
        clear_func(as_man);
        // hashmap_delete(lme_layer_objs.LME_GS_AUTH_AS, get_lme_as_enode(as_sac));
        hashmap_delete(lme_layer_objs.LME_GS_AUTH_AS, as_man);
    }
    return NULL;
}

//static void lme_enode_print_func(void *item) {
//    const dls_entity_t *node = item;
//    fprintf(stderr, "{%u}\n", node->SRC_SAC);
//}


static bool lme_map_has_ua(uint32_t target_UA) {
    size_t iter = 0;
    void *item;
    while (hashmap_iter(lme_layer_objs.LME_GS_AUTH_AS, &iter, &item)) {
        const lme_as_man_t *as_man = item;
        if (as_man->AS_UA == target_UA)
            return TRUE;
    }
    return FALSE;
}


void test_send(lme_as_man_t *as_man);


l_err entry_LME_CSCANNING(void *args);

l_err entry_LME_CONNECTING(void *args);

l_err entry_LME_AUTH(void *args);

l_err entry_LME_OPEN(void *args);


#endif //LDACS_SIM_LDACS_LME_H
