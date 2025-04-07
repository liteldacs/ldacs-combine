//
// Created by 邹嘉旭 on 2025/3/30.
//

#ifndef SNF_H
#define SNF_H

#include <ld_statemachine.h>
#include <ld_buffer.h>
#include <ld_santilizer.h>
#include <ldacs_sim.h>
#include <ld_primitive.h>
#include "ldcauc.h"
#include "snp_sub.h"
#include "net/gs_conn.h"
#include "net/net_epoll.h"

#define DEFAULT_GSNF_VERSION 0x01
#define GSNF_MSG_MAX_LEN 512
#define GTYP_LEN 4


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

typedef enum {
    GSNF_INITIAL_AS = 0xD1,
    GSNF_SNF_UPLOAD = 0x72,
    GSNF_SNF_DOWNLOAD = 0xD3,
    GSNF_KEY_TRANS = 0x75,
    GSNF_AS_AUZ_INFO = 0xB4,
    GSNF_STATE_CHANGE = 0xEE,
} GSNF_TYPE;

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

typedef int8_t (*finish_auth)();

typedef int8_t (*trans_snp)();


typedef struct snf_entity_s {
    uint32_t AS_UA;
    uint32_t GS_UA;
    uint16_t AS_SAC;
    uint16_t AS_CURR_GS_SAC; /* current connected/to connect GS SAC for AS */

    uint8_t AUTHC_MACLEN,
            AUTHC_AUTH_ID,
            AUTHC_ENC_ID,
            AUTHC_KLEN;
    sm_statemachine_t auth_fsm;
    buffer_t *shared_random;
    void *key_as_sgw_r_h;
    void *key_as_sgw_s_h;
    buffer_t *key_as_gs_b;
    void *key_as_gs_h;
    void *key_session_en_h;
    void *key_session_mac_h;

    //for GS
    bool gs_finish_auth;
    /* for SGW */
    gs_tcp_propt_t *gs_conn; // SGW -> GS

    /* for GSC */
    uint32_t gsnf_count;
} snf_entity_t;

typedef struct snf_obj_s {
    struct hashmap *snf_emap;
    snf_entity_t *as_snf_en;
    uint8_t PROTOCOL_VER;
    ldacs_roles role;
    uint16_t GS_SAC;
    net_opt_t net_opt;

    trans_snp trans_snp_func;

    //AS
    finish_auth finish_auth_func;

    //SGW
    pthread_t client_th;
    gs_tcp_propt_t *sgw_conn; // GS -> SGW

    bool is_merged;
} snf_obj_t;

extern snf_obj_t snf_obj;

typedef struct snf_args_s {
    uint8_t role;
    uint16_t AS_SAC;
    uint32_t AS_UA;
    uint16_t AS_CURR_GS_SAC;
} snf_args_t;

typedef struct ss_recv_handler_s {
    uint8_t type;

    l_err (*callback)(buffer_t *, snf_entity_t *);
} ss_recv_handler_t;


#pragma pack(1)
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


extern size_t as_recv_handlers_sz;
extern size_t sgw_recv_handlers_sz;
extern ss_recv_handler_t as_recv_handlers[];
extern ss_recv_handler_t sgw_recv_handlers[];


extern fsm_event_t ld_authc_fsm_events[];

int8_t init_as_snf_layer(finish_auth finish_auth, trans_snp trans_snp);

int8_t init_gs_snf_layer(uint16_t GS_SAC, const char *gsnf_addr, uint16_t gsnf_port, finish_auth finish_auth,
                         trans_snp trans_snp);

int8_t init_gs_snf_layer_unmerged(uint16_t GS_SAC, const char *gsnf_addr, uint16_t gsnf_port, finish_auth finish_auth,
                                  trans_snp trans_snp);

int8_t clear_snf_en(snf_entity_t *snf_en);

int8_t destory_snf_layer();

int8_t snf_LME_AUTH(void *args);

int8_t exit_LME_AUTH(void *args);

int8_t register_snf_en(snf_args_t *snf_args);

int8_t unregister_snf_en(uint16_t SAC);

/*  ss */

l_err send_auc_rqst(void *args);

l_err recv_auc_rqst(buffer_t *buf, snf_entity_t *as_man);

l_err send_auc_resp(void *args);

l_err recv_auc_resp(buffer_t *buf, snf_entity_t *as_man);

l_err send_auc_key_exec(void *args);

l_err recv_auc_key_exec(buffer_t *buf, snf_entity_t *as_man);

l_err finish_auc(void *args);

l_err send_key_update_rqst(void *args);

l_err send_key_update_rqst(void *args);

l_err recv_key_update_rqst(buffer_t *buf, snf_entity_t *as_man);

l_err send_key_update_resp(void *args);

l_err recv_key_update_resp(buffer_t *buf, snf_entity_t *as_man);

l_err send_sn_session_est_resp(void *args);

l_err recv_sn_session_est_rqst(buffer_t *buf, snf_entity_t *as_man);

l_err handle_recv_msg(buffer_t *buf, const snf_entity_t *as_man);

l_err handle_send_msg(void *args, struct_desc_t *desc, snf_entity_t *as_man, KEY_HANDLE key_med);

/* gsnf */


l_err trans_gsnf(gs_tcp_propt_t *conn, void *pkg, struct_desc_t *desc, l_err (*mid_func)(buffer_t *, void *),
                 void *args);

l_err recv_gsnf(basic_conn_t **bcp);

l_err recv_gsg(basic_conn_t **bcp);

/* define */

struct hashmap *init_enode_map();

bool has_enode_by_sac(const uint16_t as_sac);

bool has_enode_by_ua(uint32_t target_UA);

snf_entity_t *get_enode(const uint16_t as_sac);

const void *set_enode(snf_entity_t *en);

int8_t delete_enode_by_sac(uint16_t as_sac, int8_t (*clear_func)(snf_entity_t *snf_en));


#endif //SNF_H
