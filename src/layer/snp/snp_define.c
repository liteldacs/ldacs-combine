//
// Created by 邹嘉旭 on 2024/8/19.
//
#include "ldacs_snp.h"

const char *snp_ctrl_name[] = {
    "USER_PLANE_PACKET",
    "CONTROL_PLANE_PACKET"
};

const char *snp_sec_name[] = {
    "WITHOUT_MAC",
    "96_bits_MAC",
    "128_bits_MAC",
    "64_bits_MAC",
    "256_bits_MAC",
};

const char *snp_fsm_states[] = {
    "SNP_CLOSED",
    "SNP_AUTH",
    "SNP_OPEN",
};

/**
 * Snp PDU
 */
static field_desc snp_pdu_fields[] = {
    {ft_enum, 1, "CTRL", &snp_pdu_ctrl_names},
    {ft_enum, 3, "SEC_LEVEL", &snp_pdu_sec_names},
    {ft_set, 4, "NSEL", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_dl_str, 0, "DATA", NULL},
    {ft_sqn, 24, "SQN", NULL},
    {ft_end, 0, NULL, NULL},
};
struct_desc_t snp_pdu_desc = {"SNP_PDU", snp_pdu_fields};

static field_desc snp_direct_fields[] = {
    {ft_set, 12, "AS SAC", NULL},
    {ft_set, 12, "GS SAC", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_dl_str, 0, "DATA", NULL},
    {ft_end, 0, NULL, NULL},
};
struct_desc_t snp_direct_desc = {"SNP_PDU", snp_direct_fields};
