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


