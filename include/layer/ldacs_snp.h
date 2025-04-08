//
// Created by 邹嘉旭 on 2023/12/14.
//

#ifndef LDACS_SIM_LDACS_SNP_H
#define LDACS_SIM_LDACS_SNP_H
#include "layer_core.h"
#include "ldacs_lme.h"
#include "ldcauc/crypto/secure_core.h"

#define MAX_SNP_SDU_LEN 2012
#define MAX_SNP_PDU_LEN 2048
#define SNP_RANGE 10

typedef enum {
    SN_DATA_REQ = 0x1,
    SN_DATA_IND = 0x2,
    SN_UDATA_REQ = 0x3,
    SN_UDATA_IND = 0x4,
    SN_CONF_REQ = 0x5,
    SN_AUTH_REQ = 0x6,
    SN_OPEN_REQ = 0x7,
    SN_CLOSE_REQ = 0x8,
} ld_snp_prims_en;

enum SNP_CTRLS {
    USER_PLANE_PACKET = 0,
    CONTROL_PLANE_PACKET,
};

enum SNP_NSEL {
    NSEL_LME = 0x00,
    NSEL_IPV6 = 0x01,
};


extern const char *snp_ctrl_name[];
extern const char *snp_sec_name[];

typedef struct snp_layer_objs_s {
    size_t SNP_P_SDU;

    enum SEC_ALG_MACLEN SEC;
    uint32_t T_SQN;
    sm_statemachine_t snp_fsm;
} snp_layer_objs_t;

enum snp_fsm_event_type {
    SNP_EV_DEFAULT = 0,
};

enum SNP_FSM_STATES_E {
    SNP_CLOSED,
    SNP_AUTH,
    SNP_OPEN,
};

extern const char *snp_fsm_states[];

extern fsm_event_t snp_fsm_events[];

#pragma pack(1)  //结构体按一个字节对齐（不进行对齐）
typedef struct snp_pdu_s {
    uint8_t ctrl;
    uint8_t sec_level;
    uint8_t nsel;
    buffer_t *sdu;
    uint32_t sqn;
} snp_pdu_t;

#pragma pack() //恢复（一定要恢复）
extern enum_names sec_ua_names;
extern enum_names snp_pdu_ctrl_names;
extern enum_names snp_pdu_sec_names;
extern enum_names snp_fin_names;

/* 不包含MAC长度 */
#define SNP_ELES_SIZE 1+3+4+24

extern struct_desc_t snp_pdu_desc;

extern ld_prim_t SN_DATA_REQ_PRIM;
extern ld_prim_t SN_DATA_IND_PRIM;
extern ld_prim_t SN_UDATA_REQ_PRIM;
extern ld_prim_t SN_UDATA_IND_PRIM;
extern ld_prim_t SN_OPEN_REQ_PRIM;
extern ld_prim_t SN_AUTH_REQ_PRIM;
extern ld_prim_t SN_CLOSE_REQ_PRIM;
extern ld_prim_t SN_CONF_REQ_PRIM;

l_err make_snp_layer();

void init_snp_fsm(snp_layer_objs_t *snp_obj, enum SNP_FSM_STATES_E init_state);

void SN_SAPC(ld_prim_t *prim);

void SN_SAPD(ld_prim_t *prim);

void D_SAPD_cb(ld_prim_t *prim);

#endif //LDACS_SIM_LDACS_SNP_H
