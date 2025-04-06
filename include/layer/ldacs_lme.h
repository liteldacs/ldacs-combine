//
// Created by 邹嘉旭 on 2023/12/14.
//

#ifndef LDACS_SIM_LDACS_LME_H
#define LDACS_SIM_LDACS_LME_H

#include "layer_core.h"
#include "layer_rcu.h"
#include "ldacs_mac.h"
#include "ldacs_dls.h"
#include <gs_conn.h>

#define TIMER_MAX 65535
#define INIT_FL_CHN     300     /* which mapping to 1110.0 MHZ */
#define INIT_RL_CHN     8       /* which mapping to 964.0 MHZ */

#define EPLDACS_LEN 128  /* 128 bits */

#define MAX_ORDER 0xFF

#define RPSO_DELAY_INTVL  MF_TIMER
#define GSNF_MSG_MAX_LEN 512
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

enum S_TYP_E {
    AUC_RQST = 0x41,
    AUC_RESP = 0x42,
    AUC_KEY_EXC = 0x43,
    KEY_UPD_RQST = 0x44,
    KEY_UPD_RESP = 0x45,
    G_KEY_UPD_ACK = 0x46,

    FAILED_MESSAGE = 0x4F,

    SN_SESSION_EST_RQST = 0xC1,
    SN_SESSION_EST_RESP = 0xC2,
};


enum PID_E {
    PID_RESERVED = 0x0,
    PID_SIGN = 0x1,
    PID_MAC = 0x2,
    PID_BOTH = 0x3,
};

typedef enum {
    GS_SAC_RQST = 1,
    GS_SAC_RESP,
    GS_INITIAL_MSG,
    GS_SNF_UPLOAD,
    GS_SNF_DOWNLOAD,
    GS_KEY_TRANS,
    GS_HO_REQUEST,
    GS_HO_REQUEST_ACK,
    GS_HO_COMPLETE,
    GS_UP_UPLOAD_TRANSPORT,
    GS_UP_DOWNLOAD_TRANSPORT,
    GS_AS_EXIT,
} GSG_TYPE;

typedef enum {
    GSNF_INITIAL_AS = 0xD1,
    GSNF_SNF_UPLOAD = 0x72,
    GSNF_SNF_DOWNLOAD = 0xD3,
    GSNF_KEY_TRANS = 0x75,
    GSNF_AS_AUZ_INFO = 0xB4,
    GSNF_STATE_CHANGE = 0xEE,
} GSNF_TYPE;

typedef enum {
    ELE_TYP_0 = 0x0,
    ELE_TYP_1 = 0x1,
    ELE_TYP_2 = 0x2,
    ELE_TYP_3 = 0x3,
    ELE_TYP_4 = 0x4,
    ELE_TYP_5 = 0x5,
    ELE_TYP_6 = 0x6,
    ELE_TYP_7 = 0x7,
    ELE_TYP_8 = 0x8,
    ELE_TYP_9 = 0x9,
    ELE_TYP_A = 0xA,
    ELE_TYP_B = 0xB,
    ELE_TYP_C = 0xC,
    ELE_TYP_D = 0xD,
    ELE_TYP_E = 0xE,
    ELE_TYP_F = 0xF,
} GSNF_ELE_TYPE;

typedef enum {
    GSNF_ACCESS = 0x1,
    GSNF_SWITCH = 0x2,
    GSNF_EXIT = 0x3,
} GSNF_STATE;


extern const char *lme_fsm_states[];
extern const char *authc_maclen_name[];
extern const char *authc_authid_name[];
extern const char *authc_enc_name[];
extern const char *authc_klen_name[];
extern const char *ld_authc_fsm_states[];
extern const char *s_type_name[];
extern const char *pid_name[];

extern enum_names s_type_names;
extern enum_names pid_names;
extern enum_names key_type_names;
extern enum_names sib_mod_names;
extern enum_names sib_cms_names;
extern enum_names pid_names;
extern enum_names authc_maclen_names;
extern enum_names authc_auth_names;
extern enum_names authc_enc_names;
extern enum_names authc_klen_names;

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

    uint8_t AUTHC_MACLEN,
            AUTHC_AUTH_ID,
            AUTHC_ENC_ID,
            AUTHC_KLEN;
    sm_statemachine_t auth_fsm;
    ld_co_t CO;

    // for GS
    bool gs_finish_auth;

    uint8_t RPSO,
            NRPS;

    uint32_t send_T_SQN,
            recv_T_SQN;

    buffer_t *shared_random;
    KEY_HANDLE key_as_sgw_r_h;
    KEY_HANDLE key_as_sgw_s_h;
    buffer_t *key_as_gs_b;
    KEY_HANDLE key_as_gs_h;
    KEY_HANDLE key_session_en_h;
    KEY_HANDLE key_session_mac_h;

    /* for SGW */
    gs_tcp_propt_t *gs_conn; // SGW -> GS

    /* for GSC */
    uint32_t gsnf_count;
} lme_as_man_t;

lme_as_man_t *init_as_man(uint16_t AS_SAC, uint32_t AS_UA, uint16_t AS_CURR_GS_SAC,
                          enum LD_AUTHC_STATES_E init_st);

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

    pthread_t client_th;
    gs_tcp_propt_t *sgw_conn; // GS -> SGW

    /* ====================  for AS ==================== */
    lme_as_man_t *lme_as_man;

    net_opt_t net_opt;
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


typedef struct ss_recv_handler_s {
    uint8_t type;

    l_err (*callback)(buffer_t *, lme_as_man_t *);
} ss_recv_handler_t;

typedef struct gsnf_pkt_format_desc_s {
    GSNF_TYPE G_TYPE;
    GSNF_ELE_TYPE ELE_TYPE;
    struct_desc_t *f_desc;
    size_t struct_size;

    void (*malloc_cb)(void *);
} gsnf_pkt_format_desc_t;

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


typedef struct auc_sharedinfo_s {
    uint8_t MAC_LEN;
    uint8_t AUTH_ID;
    uint8_t ENC_ID;
    uint16_t AS_SAC;
    uint16_t GS_SAC;
    uint8_t K_LEN;
    buffer_t *N_2;
} auc_sharedinfo_t;

typedef struct auc_rqst_s {
    uint8_t S_TYP;
    uint8_t VER;
    uint8_t PID;
    uint16_t AS_SAC;
    uint16_t GS_SAC;
    uint8_t MAC_LEN;
    uint8_t AUTH_ID;
    uint8_t ENC_ID;
    buffer_t *N_1;
} auc_rqst_t;

typedef struct auc_resp_s {
    uint8_t S_TYP;
    uint8_t VER;
    uint8_t PID;
    uint16_t AS_SAC;
    uint16_t GS_SAC;
    uint8_t MAC_LEN;
    uint8_t AUTH_ID;
    uint8_t ENC_ID;
    uint8_t K_LEN;
    buffer_t *N_2;
} auc_resp_t;

typedef struct key_upd_rqst_s {
    uint8_t S_TYP;
    uint8_t VER;
    uint8_t PID;
    uint16_t AS_SAC;
    uint8_t KEY_TYPE;
    uint16_t SAC_src;
    uint16_t SAC_dst;
    uint16_t NCC;
    buffer_t *NONCE;
} key_upd_rqst_t;

typedef struct key_upd_resp_s {
    uint8_t S_TYP;
    uint8_t VER;
    uint8_t PID;
    uint16_t AS_SAC;
    uint8_t KEY_TYPE;
    uint16_t SAC_dst;
    uint16_t NCC;
} key_upd_resp_t;


typedef struct auc_key_exec_s {
    uint8_t S_TYP;
    uint8_t VER;
    uint8_t PID;
    uint16_t AS_SAC;
    uint16_t GS_SAC;
    uint8_t MAC_LEN;
    uint8_t AUTH_ID;
    uint8_t ENC_ID;
    buffer_t *N_3;
} auc_key_exec_t;


typedef struct sn_session_est_rqst_s {
    uint8_t SN_TYP;
    uint8_t VER;
    uint8_t PID;
    uint16_t AS_SAC;
    uint8_t SER_TYPE;
} sn_session_est_rqst_t;


typedef struct sn_session_est_resp_s {
    uint8_t SN_TYP;
    uint8_t VER;
    uint8_t PID;
    uint16_t AS_SAC;
    buffer_t *IP_AS;
} sn_session_est_resp_t;

typedef struct failed_message_s {
    uint8_t SN_TYP;
    uint8_t VER;
    uint8_t PID;
    uint16_t AS_SAC;
    uint8_t FAILED_TYPE;
    buffer_t *msg;
} failed_message_t;

typedef struct gsg_sac_pkt_s {
    uint8_t TYPE;
    uint32_t UA;
    buffer_t *sdu;
} gsg_sac_pkt_t;

/**
 * TODO： 12.17需改： 除了前两条报文单独解析外，其他报文都是一个head + payload的形式，head共用一个描述，payload各自不同，switch的时候把他们放一起，查出来SAC后对应的as_man后再搞一次switch
 */
typedef struct gsg_pkt_s {
    uint8_t TYPE;
    uint16_t AS_SAC;
    buffer_t *sdu;
} gsg_pkt_t;

typedef struct gsg_as_exit_s {
    uint8_t TYPE;
    uint16_t AS_SAC;
} gsg_as_exit_t;

typedef struct gsnf_pkt_cn_s {
    uint8_t G_TYP;
    uint8_t VER;
    uint16_t AS_SAC;
    uint8_t ELE_TYPE;
    buffer_t *sdu;
} gsnf_pkt_cn_t;

typedef struct gsnf_pkt_cn_ini_s {
    uint8_t G_TYP;
    uint8_t VER;
    uint16_t AS_SAC;
    uint8_t ELE_TYPE;
    uint32_t UA;
    buffer_t *sdu;
} gsnf_pkt_cn_ini_t;

typedef struct gsnf_as_auz_info_s {
    uint8_t G_TYP;
    uint8_t VER;
    uint16_t AS_SAC;
    uint8_t is_legal;
    uint8_t auz_type;
} gsnf_as_auz_info_t;

typedef struct gsnf_st_chg_s {
    uint8_t G_TYP;
    uint8_t VER;
    uint16_t AS_SAC;
    uint8_t State;
    uint16_t GS_SAC;
} gsnf_st_chg_t;

typedef struct gs_key_trans_s {
    buffer_t *key;
    buffer_t *nonce;
} gs_key_trans_t;

typedef struct gs_sac_resp_sdu_s {
    uint16_t SAC;
} gs_sac_resp_sdu_t;


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
extern struct_desc_t auc_sharedinfo_desc;
extern struct_desc_t auc_rqst_desc;
extern struct_desc_t auc_resp_desc;
extern struct_desc_t auc_key_exec_desc;
extern struct_desc_t key_upd_rqst_desc;
extern struct_desc_t key_upd_resp_desc;
extern struct_desc_t sn_session_est_rqst_desc;
extern struct_desc_t sn_session_est_resp_desc;
extern struct_desc_t failed_message_desc;
extern struct_desc_t gsg_sac_pkt_desc;
extern struct_desc_t gsg_pkt_desc;
extern struct_desc_t gsg_as_exit_desc;
extern struct_desc_t gsnf_pkt_cn_desc;
extern struct_desc_t gsnf_pkt_cn_ini_desc;
extern struct_desc_t gsnf_as_auz_info_desc;
extern struct_desc_t gsnf_st_chg_desc;
extern struct_desc_t gs_sac_resp_desc;
extern struct_desc_t gs_key_trans_desc;


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

extern size_t as_recv_handlers_sz;
extern size_t sgw_recv_handlers_sz;
extern ss_recv_handler_t as_recv_handlers[];
extern ss_recv_handler_t sgw_recv_handlers[];
extern fsm_event_t ld_authc_fsm_events[];
extern struct sm_state_s ld_authc_states[];

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

l_err change_LME_OPEN();

void exit_LME_CONN_OPEN_action(void *curr_st_data, struct sm_event_s *event, void *new_state_data);

/* mms */
l_err init_lme_mms(lme_layer_objs_t *obj);

void *trans_lme_bc_timer_func(void *args);

void trans_bc_acb_func(void *args);

void trans_bc_sib_func(void *args);

void trans_bc_mac_timer_func(evutil_socket_t fd, short event, void *arg);

void *trans_ra_cr_timer_func(void *args);

l_err start_mms();

void free_CO(uint16_t co);

/* ss */
l_err init_lme_ss(lme_layer_objs_t *obj);

l_err entry_LME_AUTH(void *);

l_err send_auc_rqst(void *args);

l_err recv_auc_rqst(buffer_t *buf, lme_as_man_t *as_man);

l_err send_auc_resp(void *args);

l_err recv_auc_resp(buffer_t *buf, lme_as_man_t *as_man);

l_err send_auc_key_exec(void *args);

l_err recv_auc_key_exec(buffer_t *buf, lme_as_man_t *as_man);

l_err finish_auc(void *args);

l_err send_key_update_rqst(void *args);

l_err recv_key_update_rqst(buffer_t *buf, lme_as_man_t *as_man);

l_err send_key_update_resp(void *args);

l_err recv_key_update_resp(buffer_t *buf, lme_as_man_t *as_man);

l_err send_sn_session_est_resp(void *args);

l_err recv_sn_session_est_rqst(buffer_t *buf, lme_as_man_t *as_man);

l_err handle_recv_msg(buffer_t *buf, const lme_as_man_t *as_man);

l_err handle_send_msg(void *args, struct_desc_t *desc, lme_as_man_t *as_man, KEY_HANDLE key_med);

/* rms */
l_err init_lme_rms(lme_layer_objs_t *obj);

// l_err lme_req_register(res_req_t *res_req);

void *trans_lme_cc_timer_func(void *args);

void trans_cc_sd_timer_func(void *args);

void trans_cc_dd_func(void *args);

void trans_cc_mac_timer_func(void *args);

void rrm_timer_func(void *args);

/* gsnf */


// l_err trans_gsnf(buffer_t *sdu, uint16_t as_sac, gs_tcp_propt_t *conn, GSNF_TYPE g_type);
l_err trans_gsnf(gs_tcp_propt_t *conn, void *pkg, struct_desc_t *desc, l_err (*mid_func)(buffer_t *, void *),
                 void *args);

// l_err sgw_recv_gsnf(gs_tcp_propt_t *mlt_ld);

l_err recv_gsnf(basic_conn_t **bcp);

l_err recv_gsg(basic_conn_t **bcp);

/* auth */
buffer_t *get_auc_sharedinfo_buf(auc_sharedinfo_t *info);

l_err generate_auc_kdf(buffer_t *random, KEY_HANDLE*key_as_sgw, KEY_HANDLE*key_as_gs,
                       buffer_t **key_as_gs_raw);


/* lme util */

void init_lme_fsm(lme_layer_objs_t *lme_obj, enum LME_FSM_STATES_E init_state);

static uint64_t hash_lme_as_enode(const void *item, uint64_t seed0, uint64_t seed1) {
    const lme_as_man_t *node = item;
    return hashmap_sip(&node->AS_SAC, sizeof(uint16_t), seed0, seed1);
}

static lme_as_man_t *get_lme_as_enode(const uint16_t as_sac) {
    return hashmap_get(lme_layer_objs.LME_GS_AUTH_AS, &(lme_as_man_t){.AS_SAC = as_sac});
}

static const bool has_lme_as_enode(const uint16_t as_sac) {
    return hashmap_get(lme_layer_objs.LME_GS_AUTH_AS, &(lme_as_man_t){.AS_SAC = as_sac}) != NULL;
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
