//
// Created by 邹嘉旭 on 2024/8/18.
//
#include "ldacs_lme.h"


const char *lme_fsm_states[] = {
    "LME_FSCANNING",
    "LME_CSCANNING",
    "LME_CONNECTING",
    "LME_AUTH",
    "LME_OPEN",
};
const char *s_type_name[] = {
    "AUC_RQST",
    "AUC_RESP",
    "AUC_KEY_EXC",
    "KEY_UPD_RQST",
    "KEY_UPD_RESP",
    "G_KEY_UPD_ACK",

    "SN_SESSION_EST_RQST",
    "SN_SESSION_EST_RESP",
};

const char *pid_name[] = {
    "PID_RESERVED",
    "PID_SIGN",
    "PID_MAC",
    "PID_BOTH",
};

const char *authc_maclen_name[] = {
    "AUTHC_MACLEN_INVALID",
    "AUTHC_MACLEN_96",
    "AUTHC_MACLEN_128",
    "AUTHC_MACLEN_64",
    "AUTHC_MACLEN_256",
};

const char *authc_authid_name[] = {
    "AUTHC_AUTH_INVALID",
    "AUTHC_AUTH_SM3HMAC",
    "AUTHC_AUTH_SM2_WITH_SM3",
};

const char *authc_enc_name[] = {
    "AUTHC_ENC_INVALID",
    "AUTHC_ENC_SM4_CBC",
    "AUTHC_ENC_SM4_CFB",
    "AUTHC_ENC_SM4_OFB",
    "AUTHC_ENC_SM4_ECB",
    "AUTHC_ENC_SM4_CTR",
};

const char *authc_klen_name[] = {
    "AUTHC_KLEN_RESERVED",
    "AUTHC_KLEN_128",
    "AUTHC_KLEN_256",
};

const char *ld_authc_fsm_states[] = {
    "LD_AUTHC_A0",
    "LD_AUTHC_A1",
    "LD_AUTHC_A2",
    "LD_AUTHC_G0",
    "LD_AUTHC_G1",
    "LD_AUTHC_G2",
};


enum_names s_type_names = {AUC_RQST, SN_SESSION_EST_RESP, s_type_name, NULL};
enum_names pid_names = {PID_RESERVED, PID_BOTH, pid_name, NULL};
enum_names key_type_names = {ROOT_KEY, GROUP_KEY_CC, type_names, NULL};
enum_names authc_maclen_names = {AUTHC_MACLEN_INVALID, AUTHC_MACLEN_256, authc_maclen_name, NULL};
enum_names authc_auth_names = {AUTHC_AUTH_INVALID, AUTHC_AUTH_SM2_WITH_SM3, authc_authid_name, NULL};
enum_names authc_enc_names = {AUTHC_ENC_INVALID, AUTHC_ENC_SM4_CTR, authc_enc_name, NULL};
enum_names authc_klen_names = {AUTHC_KLEN_128, AUTHC_KLEN_256, authc_klen_name, NULL};


static field_desc bc_acb_fields[] = {
    {ft_set, B_TYPE_LEN, "B_TYPE", NULL},
    {ft_set, 12, "SAC", NULL},
    {ft_set, 9, "FLF", NULL},
    {ft_set, 9, "RLF", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_crc, CRC_8_SIZE, "CRC", NULL},
    {ft_end, 0, NULL, NULL},
};
struct_desc_t bc_acb_desc = {"ACB_DESC", bc_acb_fields};

/**
 * BC SIB
 */
static field_desc bc_sib_fields[] = {
    {ft_set, B_TYPE_LEN, "B_TYPE", NULL},
    {ft_set, 12, "SAC", NULL},
    {ft_set, 3, "VER", NULL},
    {ft_set, 9, "FLF", NULL},
    {ft_set, 9, "RLF", NULL},
    {ft_enum, 1, "MOD", &sib_mod_names},
    {ft_enum, 3, "CMS", &sib_cms_names},
    {ft_set, 7, "EIRP", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_crc, CRC_8_SIZE, "CRC", NULL},
    {ft_end, 0, NULL, NULL},
};

struct_desc_t bc_sib_desc = {"SIB_DESC", bc_sib_fields};

/**
 * BC MAC
 */
static field_desc bc_hmac_fields[] = {
    {ft_set, B_TYPE_LEN, "B_TYPE", NULL},
    {ft_dl_str, 0, "BC_HMAC", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_crc, CRC_8_SIZE, "CRC", NULL},
    {ft_end, 0, NULL, NULL},
};

struct_desc_t bc_hmac_desc = {"BC_HMAC_DESC", bc_hmac_fields};

static field_desc cc_slot_desc_fields[] = {
    {ft_set, 3, "CCL", NULL},
    {ft_set, 5, "DCL", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_crc, CRC_8_SIZE, "CRC", NULL},
    {ft_end, 0, NULL, NULL},
};

struct_desc_t cc_slot_desc = {"SLOT_DESC", cc_slot_desc_fields};

static field_desc cc_dcch_desc_fields[] = {
    {ft_set, C_TYPE_LEN, "C_TYPE", NULL},
    {ft_set, 5, "COL", NULL},
    {ft_set, 9, "COS", NULL},
    {ft_set, 9, "COM", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_crc, CRC_8_SIZE, "CRC", NULL},
    {ft_end, 0, NULL, NULL},
};

struct_desc_t cc_dcch_desc = {"DCCH_DESC", cc_dcch_desc_fields};


/**
 * CC cell response
 */
static field_desc cc_cell_resp_fields[] = {
    {ft_set, C_TYPE_LEN, "C_TYPE", NULL},
    {ft_set, 12, "SAC", NULL},
    {ft_set, 28, "UA", NULL},
    {ft_set, 7, "PAV", NULL},
    {ft_set, 10, "FAV", NULL},
    {ft_set, 10, "TAV", NULL},
    {ft_set, 9, "CO", NULL},
    {ft_fl_str, 0, "EPLDACS", &(pk_fix_length_t){.len = EPLDACS_LEN >> 3}},
    {ft_set, 16, "CCLDACS",NULL},
    {ft_set, 3, "VER", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_crc, CRC_8_SIZE, "CRC", NULL},
    {ft_end, 0, NULL, NULL},
};
struct_desc_t cc_cell_resp_desc = {"CC_CELL_RESP", cc_cell_resp_fields};

/**
 * CC FL ALLOC
 */
static field_desc cc_fl_alloc_fields[] = {
    {ft_set, C_TYPE_LEN, "C_TYPE", NULL},
    {ft_set, 12, "SAC", NULL},
    {ft_set, 14, "BO", NULL},
    {ft_set, 14, "BL", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_crc, CRC_8_SIZE, "CRC", NULL},
    {ft_end, 0, NULL, NULL},
};
struct_desc_t cc_fl_alloc_desc = {"CC_FL_ALLOC", cc_fl_alloc_fields};

/**
 * CC RL ALLOC
 */
static field_desc cc_rl_alloc_fields[] = {
    {ft_set, C_TYPE_LEN, "C_TYPE", NULL},
    {ft_set, 12, "SAC", NULL},
    {ft_set, 8, "RPSO", NULL},
    {ft_set, 8, "NRPS", NULL},
    {ft_set, 3, "CMS", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_crc, CRC_8_SIZE, "CRC", NULL},
    {ft_end, 0, NULL, NULL},
};
struct_desc_t cc_rl_alloc_desc = {"CC_RL_ALLOC", cc_rl_alloc_fields};

/**
 * CC MAC
 */
static field_desc cc_hmac_fields[] = {
    {ft_set, C_TYPE_LEN, "C_TYPE", NULL},
    {ft_dl_str, 0, "CC_HMAC", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_crc, CRC_8_SIZE, "CRC", NULL},
    {ft_end, 0, NULL, NULL},
};

struct_desc_t cc_hmac_desc = {"CC_HMAC_DESC", cc_hmac_fields};

/**
 * RA cell request
 */
static field_desc ra_cell_rqst_fields[] = {
    {ft_set, R_TYPE_LEN, "R_TYPE", NULL},
    {ft_set, 28, "UA", NULL},
    {ft_set, 12, "SAC", NULL},
    {ft_set, 1, "SCGS", NULL},
    {ft_set, 3, "VER", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_crc, CRC_8_SIZE, "CRC", NULL},
    {ft_end, 0, NULL, NULL},
};
struct_desc_t ra_cell_rqst_desc = {"RA_CELL_REQ_DESC", ra_cell_rqst_fields};

/**
 * DC KEEP ALIVE
 */
static field_desc dc_keep_alive_fields[] = {
    {ft_set, 4, "D_TYP", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_end, 0, NULL, NULL},
};

struct_desc_t dc_keep_alive_desc = {"DC_KEEP_ALIVE", dc_keep_alive_fields};

/**
 * DC RSC RQST
 */
static field_desc dc_rsc_rqst_fields[] = {
    {ft_set, 4, "D_TYP", NULL},
    {ft_set, 3, "SC", NULL},
    {ft_set, 15, "REQ", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_end, 0, NULL, NULL},
};

struct_desc_t dc_rsc_rqst_desc = {"DC_RSC_RQST", dc_rsc_rqst_fields};

/**
 * DC CELL EXIT
 */
static field_desc dc_cell_exit_fields[] = {
    {ft_set, 4, "D_TYP", NULL},
    {ft_set, 12, "SAC", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_end, 0, NULL, NULL},
};

struct_desc_t dc_cell_exit_desc = {"DC_RSC_RQST", dc_cell_exit_fields};

/**
 * Sharedinfo
 */
static field_desc auc_sharedinfo_fields[] = {
    {ft_enum, AUTHC_ALG_S_LEN, "MAC_LEN", &authc_maclen_names},
    {ft_enum, AUTHC_ALG_S_LEN, "AUTH_ID", &authc_auth_names},
    {ft_enum, AUTHC_ALG_S_LEN, "ENC_ID", &authc_enc_names},
    {ft_set, SAC_LEN, "AS_SAC", NULL},
    {ft_set, SAC_LEN, "GS_SAC", NULL},
    {ft_enum, AUTHC_KLEN_LEN, "KLEN", &authc_klen_names},
    {ft_pad, 0, "PAD", NULL},
    {ft_fl_str, 0, "N2", &(pk_fix_length_t){.len = NONCE_LEN}},
    {ft_end, 0, NULL, NULL},
};
struct_desc_t auc_sharedinfo_desc = {"AUC_SHAREDINFO", auc_sharedinfo_fields};

/**
 * AUC-RQST
 */
static field_desc auc_rqst_fields[] = {
    {ft_enum, S_TYP_LEN, "S_TYP", &s_type_names},
    {ft_set, VER_LEN, "VER", NULL},
    {ft_enum, PID_LEN, "PID", &pid_names},
    {ft_set, SAC_LEN, "AS_SAC", NULL},
    {ft_set, SAC_LEN, "GS_SAC", NULL},
    {ft_enum, AUTHC_ALG_S_LEN, "MAC_LEN", &authc_maclen_names},
    {ft_enum, AUTHC_ALG_S_LEN, "AUTH_ID", &authc_auth_names},
    {ft_enum, AUTHC_ALG_S_LEN, "ENC_ID", &authc_enc_names},
    {ft_fl_str, 0, "N1", &(pk_fix_length_t){.len = NONCE_LEN}},
    {ft_pad, 0, "PAD", NULL},
    {ft_end, 0, NULL, NULL},
};

struct_desc_t auc_rqst_desc = {"AUC_RQST", auc_rqst_fields};
/**
 * AUC-RESP
 */
static field_desc auc_resp_fields[] = {
    {ft_enum, S_TYP_LEN, "S_TYP", &s_type_names},
    {ft_set, VER_LEN, "VER", NULL},
    {ft_enum, PID_LEN, "PID", &pid_names},
    {ft_set, SAC_LEN, "AS_SAC", NULL},
    {ft_set, SAC_LEN, "GS_SAC", NULL},
    {ft_enum, AUTHC_ALG_S_LEN, "MAC_LEN", &authc_maclen_names},
    {ft_enum, AUTHC_ALG_S_LEN, "AUTH_ID", &authc_auth_names},
    {ft_enum, AUTHC_ALG_S_LEN, "ENC_ID", &authc_enc_names},
    {ft_enum, AUTHC_KLEN_LEN, "KLEN", &authc_klen_names},
    {ft_pad, 0, "PAD", NULL},
    {ft_fl_str, 0, "N2", &(pk_fix_length_t){.len = NONCE_LEN}},
    {ft_end, 0, NULL, NULL},
};

struct_desc_t auc_resp_desc = {"AUC_RESP", auc_resp_fields};
/**
 * AUC-KEY-EXEC
 */
static field_desc auc_key_exec_fields[] = {
    {ft_enum, S_TYP_LEN, "S_TYP", &s_type_names},
    {ft_set, VER_LEN, "VER", NULL},
    {ft_enum, PID_LEN, "PID", &pid_names},
    {ft_set, SAC_LEN, "AS_SAC", NULL},
    {ft_set, SAC_LEN, "GS_SAC", NULL},
    {ft_enum, AUTHC_ALG_S_LEN, "MAC_LEN", &authc_maclen_names},
    {ft_enum, AUTHC_ALG_S_LEN, "AUTH_ID", &authc_auth_names},
    {ft_enum, AUTHC_ALG_S_LEN, "ENC_ID", &authc_enc_names},
    {ft_fl_str, 0, "N3", &(pk_fix_length_t){.len = NONCE_LEN}},
    {ft_pad, 0, "PAD", NULL},
    {ft_end, 0, NULL, NULL},
};
struct_desc_t auc_key_exec_desc = {"AUC_KEY_EXEC", auc_key_exec_fields};

static field_desc key_upd_rqst_fields[] = {
    {ft_enum, S_TYP_LEN, "S_TYP", &s_type_names},
    {ft_set, VER_LEN, "VER", NULL},
    {ft_enum, PID_LEN, "PID", &pid_names},
    {ft_set, SAC_LEN, "AS_SAC", NULL},
    {ft_enum, KEY_TYPE_LEN, "KEY_TYPE", &key_type_names},
    {ft_set, SAC_LEN, "GS_SAC_SRC", NULL},
    {ft_set, SAC_LEN, "GS_SAC_DST", NULL},
    {ft_set, NCC_LEN, "NCC", NULL},
    {ft_fl_str, 0, "NONCE", &(pk_fix_length_t){.len = NONCE_LEN}},
    {ft_pad, 0, "PAD", NULL},
    {ft_end, 0, NULL, NULL},
};
struct_desc_t key_upd_rqst_desc = {"KEY_UPDATE_REQUEST", key_upd_rqst_fields};

static field_desc key_upd_resp_fields[] = {
    {ft_enum, S_TYP_LEN, "S_TYP", &s_type_names},
    {ft_set, VER_LEN, "VER", NULL},
    {ft_enum, PID_LEN, "PID", &pid_names},
    {ft_set, SAC_LEN, "AS_SAC", NULL},
    {ft_enum, KEY_TYPE_LEN, "KEY_TYPE", &key_type_names},
    {ft_set, SAC_LEN, "GS_SAC_DST", NULL},
    {ft_set, NCC_LEN, "NCC", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_end, 0, NULL, NULL},
};

struct_desc_t key_upd_resp_desc = {"KEY_UPDATE_RESPONSE", key_upd_resp_fields};

static field_desc sn_session_est_rqst_fields[] = {
    {ft_set, 8, "SN_TYP", NULL},
    {ft_set, 3, "VER", NULL},
    {ft_set, 2, "PID", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_set, 12, "SAC", NULL},
    {ft_set, 4, "SERVICE TYPE", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_end, 0, NULL, NULL},
};

struct_desc_t sn_session_est_rqst_desc = {"SN_SESSION_EST_RQST", sn_session_est_rqst_fields};

static field_desc sn_session_est_resp_fields[] = {
    {ft_set, 8, "SN_TYP", NULL},
    {ft_set, 3, "VER", NULL},
    {ft_set, 2, "PID", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_set, 12, "SAC", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_fl_str, 0, "IP", &(pk_fix_length_t){.len = IPV6_ADDRLEN >> 3}},
    {ft_pad, 0, "PAD", NULL},
    {ft_end, 0, NULL, NULL},
};

struct_desc_t sn_session_est_resp_desc = {"SN_SESSION_EST_RESP", sn_session_est_resp_fields};

static field_desc failed_message_fields[] = {
    {ft_set, 8, "SN_TYP", NULL},
    {ft_set, 3, "VER", NULL},
    {ft_set, 2, "PID", NULL},
    {ft_set, 12, "SAC", NULL},
    {ft_set, 4, "FAILED TYPE", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_dl_str, 0, "MSG", NULL},
    {ft_end, 0, NULL, NULL},
};

struct_desc_t failed_message_desc = {"FAILED_MESSGAE", failed_message_fields};

static field_desc gsg_sac_pkt_fields[] = {
    {ft_set, 4, "TYPE", NULL},
    {ft_set, 28, "UA", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_dl_str, 0, "SDU", NULL},
    {ft_end, 0, NULL, NULL},
};

struct_desc_t gsg_sac_pkt_desc = {"GSG SAC PKT", gsg_sac_pkt_fields};

static field_desc gsg_pkt_fields[] = {
    {ft_set, 4, "TYPE", NULL},
    {ft_set, 12, "SAC", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_dl_str, 0, "SDU", NULL},
    {ft_end, 0, NULL, NULL},
};
struct_desc_t gsg_pkt_desc = {"GSG PKT", gsg_pkt_fields};

static field_desc gsg_as_exit_fields[] = {
    {ft_set, 4, "TYPE", NULL},
    {ft_set, 12, "AS SAC", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_end, 0, NULL, NULL},
};
struct_desc_t gsg_as_exit_desc = {"GSNF STATE CHANGE", gsg_as_exit_fields};

static field_desc gsnf_pkt_cn_fields[] = {
    {ft_set, 8, "G_TYP", NULL},
    {ft_set, 4, "VER", NULL},
    {ft_set, 12, "SAC", NULL},
    {ft_set, 4, "ELE_TYPE", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_dl_str, 0, "SDU", NULL},
    {ft_end, 0, NULL, NULL},
};
struct_desc_t gsnf_pkt_cn_desc = {"GSNF PKT", gsnf_pkt_cn_fields};

static field_desc gsnf_pkt_cn_ini_fields[] = {
    {ft_set, 8, "G_TYP", NULL},
    {ft_set, 4, "VER", NULL},
    {ft_set, 12, "SAC", NULL},
    {ft_set, 4, "ELE_TYPE", NULL},
    {ft_set, 28, "UA", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_dl_str, 0, "SDU", NULL},
    {ft_end, 0, NULL, NULL},
};
struct_desc_t gsnf_pkt_cn_ini_desc = {"GSNF INITIAL AS PKT", gsnf_pkt_cn_ini_fields};

static field_desc gsnf_as_auz_info_fields[] = {
    {ft_set, 8, "G_TYP", NULL},
    {ft_set, 4, "VER", NULL},
    {ft_set, 12, "SAC", NULL},
    {ft_set, 4, "IS LEGAL", NULL},
    {ft_set, 4, "AUZ TYPE", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_end, 0, NULL, NULL},
};
struct_desc_t gsnf_as_auz_info_desc = {"GSNF AS AUZ INFO PKT", gsnf_as_auz_info_fields};


static field_desc gsnf_st_chg_fields[] = {
    {ft_set, 8, "G_TYP", NULL},
    {ft_set, 4, "VER", NULL},
    {ft_set, 12, "AS SAC", NULL},
    {ft_set, 4, "STATE", NULL},
    {ft_set, 12, "GS SAC", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_end, 0, NULL, NULL},
};
struct_desc_t gsnf_st_chg_desc = {"GSNF STATE CHANGE", gsnf_st_chg_fields};


static field_desc gs_sac_resp_fields[] = {
    {ft_set, 12, "SAC", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_end, 0, NULL, NULL},
};

struct_desc_t gs_sac_resp_desc = {"GS SAC RESP", gs_sac_resp_fields};

static field_desc gs_key_trans_fields[] = {
    {ft_dl_str, 0, "KEY", NULL},
    {ft_dl_str, 0, "NONCE", NULL},
    {ft_end, 0, NULL, NULL},
};

struct_desc_t gs_key_trans_desc = {"KEY_TRANS_DESC", gs_key_trans_fields};

ld_format_desc_t bc_format_descs[] = {
    {0, NULL, 0},
    {B_TYP_ACB, &bc_acb_desc, 6, sizeof(bc_acb_t), free},
    {B_TYP_SIB, &bc_sib_desc, 7, sizeof(bc_sib_t), free},
    {B_TYP_STB, NULL, 0},
    {0, NULL, 0},
    {B_TYP_GS_POS_B, NULL, 0},
    {B_TYP_BC_MAC, &bc_hmac_desc, 0},
    {B_TYP_SF_NUMBER, NULL, 0},
};

ld_format_desc_t cc_format_descs[] = {
    {0, NULL, 0},
    {C_TYP_DCCH_DESC, &cc_dcch_desc, 5, sizeof(cc_dcch_desc_t), free, 1, DIR_LME},
    {C_TYP_CMS_FL, &cc_dcch_desc, 5, sizeof(cc_dcch_desc_t), free, 2, DIR_LME},
    {C_TYP_DCCH_POLL, &cc_dcch_desc, 5, sizeof(cc_dcch_desc_t), free, 2, DIR_LME},
    {C_TYP_CELL_RESP, &cc_cell_resp_desc, 30, sizeof(cc_cell_resp_t), free_cc_cell_resp, 3, DIR_LME},
    {C_TYP_CHANGE_CO, &cc_dcch_desc, 5, sizeof(cc_dcch_desc_t), free, 3, DIR_LME},
    {C_TYP_CELL_DENIED, &cc_dcch_desc, 5, sizeof(cc_dcch_desc_t), free, 3, DIR_LME},
    {C_TYP_LM_DATA, &cc_dcch_desc, 5, sizeof(cc_dcch_desc_t), free, 4, DIR_LME},
    {0, NULL, 0},
    {C_TYP_ACK, &cc_ack_desc, 6, sizeof(cc_ack_t), free, 5, DIR_DLS},
    {C_TYP_ACK_FRAG, &cc_frag_ack_desc, 6, sizeof(cc_frag_ack_t), free, 6, DIR_DLS},
    {C_TYP_FL_ALLOC, &cc_fl_alloc_desc, 7, sizeof(cc_fl_alloc_t), free, 7, DIR_DLS},
    {C_TYP_RL_ALLOC, &cc_rl_alloc_desc, 6, sizeof(cc_rl_alloc_t), free, 8, DIR_DLS},
    {C_TYP_P_RL_ALLOC, &cc_dcch_desc, 5, sizeof(cc_dcch_desc_t), free, 8, DIR_LME},
    {C_TYP_SYNC_POLL, &cc_dcch_desc, 5, sizeof(cc_dcch_desc_t), free, 9, DIR_LME},
    {C_TYP_HO_COM, &cc_dcch_desc, 5, sizeof(cc_dcch_desc_t), free, 10, DIR_LME},
    {C_TYP_KEEY_ALIVE, &cc_dcch_desc, 5, sizeof(cc_dcch_desc_t), free, 11, DIR_LME},
    {C_TYP_P_RL_CANCEL, &cc_dcch_desc, 5, sizeof(cc_dcch_desc_t), free, 8, DIR_LME},
    {C_TYP_CC_MAC, &cc_hmac_desc, 0, sizeof(cc_mac_bd_t), free, MAX_ORDER, DIR_LME},
    {C_TYP_SLOT_DESC, &cc_slot_desc, 2, sizeof(cc_slot_desc_t), free, 0, DIR_MAC},
};
ld_format_desc_t ra_format_descs[] = {
    {0, NULL, 0},
    {R_TYP_CR, &ra_cell_rqst_desc, 7, sizeof(ra_cell_rqst_t), free},
    {0, NULL, 0},
    {0, NULL, 0},
};

ld_format_desc_t dc_format_descs[] = {
    {DC_TYP_PADDING, NULL, 0},
    {DC_TYP_POW_REP, &cc_dcch_desc, 5, sizeof(cc_dcch_desc_t), free, 1, DIR_LME},
    {0, NULL, 0},
    {DC_TYP_ACK, &dc_ack_desc, 4, sizeof(dc_ack_desc), free, 2, DIR_DLS},
    {DC_TYP_ACK_FRAG, &dc_frag_ack_desc, 3, sizeof(dc_frag_ack_t), free, 3, DIR_DLS},
    {DC_TYP_CELL_EXIT, &dc_cell_exit_desc, 2, sizeof(dc_cell_exit_t), free, 3, DIR_LME},
    {DC_TYP_PRSC_RQST_CANCEL, &cc_dcch_desc, 5, sizeof(cc_dcch_desc_t), free, 4, DIR_DLS},
    {DC_TYP_RSC_RQST, &dc_rsc_rqst_desc, 3, sizeof(dc_rsc_rqst_t), free, 5, DIR_LME},
    {DC_TYP_PRSC_RQST, &cc_dcch_desc, 5, sizeof(cc_dcch_desc_t), free, 5, DIR_DLS},
    {DC_TYP_KEEP_ALIVE, &dc_keep_alive_desc, 1, sizeof(dc_keep_alive_t), free, 6, DIR_LME},
    {DC_TYP_VCS, &cc_dcch_desc, 5, sizeof(cc_dcch_desc_t), free, 4, DIR_VI},
    {DC_TYP_VTN, &cc_dcch_desc, 5, sizeof(cc_dcch_desc_t), free, 2, DIR_VI},
    {0, NULL, 0},
    {0, NULL, 0},
    {0, NULL, 0},
    {0, NULL, 0},
};


size_t as_recv_handlers_sz = 3;
ss_recv_handler_t as_recv_handlers[] = {
    {AUC_RESP, recv_auc_resp},
    {KEY_UPD_RQST, recv_key_update_rqst},
    {SN_SESSION_EST_RQST, recv_sn_session_est_rqst},
};

size_t sgw_recv_handlers_sz = 3;
ss_recv_handler_t sgw_recv_handlers[] = {
    {AUC_RQST, recv_auc_rqst},
    {AUC_KEY_EXC, recv_auc_key_exec},
    {KEY_UPD_RESP, recv_key_update_resp},
};


fsm_event_t ld_authc_fsm_events[] = {
    {"LD_AUTHC_A0", NULL, NULL},
    {"LD_AUTHC_A1", send_auc_rqst, NULL},
    {"LD_AUTHC_A2", send_auc_key_exec, NULL},
    {"LD_AUTHC_G0", NULL, NULL},
    {"LD_AUTHC_G1", send_auc_resp, NULL},
    {"LD_AUTHC_G2", finish_auc, NULL},
};
