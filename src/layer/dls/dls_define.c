//
// Created by 邹嘉旭 on 2024/8/19.
//
#include "ldacs_dls.h"
/**
 * DLS DATA
 */
static field_desc dls_data_fields[] = {
    {ft_set, DATA_TYPE_LEN, "TYP", NULL},
    {ft_set, 1, "RST", NULL},
    {ft_set, 1, "LFR", NULL},
    {ft_set, 3, "SC", NULL},
    {ft_set, 5, "PID", NULL},
    {ft_set, 11, "SEQ2", NULL},
    {ft_plen, 11, "LEN", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_p_raw, 0, "DATA", &dls_data_crc_size},
    {ft_crc, CRC_32_SIZE, "CRC", NULL},
    {ft_end, 0, NULL, NULL},
};

struct_desc_t dls_data_desc = {"DLS_DATA_DESC", dls_data_fields};
/**
 * CC ACK
 */
static field_desc cc_ack_fields[] = {
    {ft_set, C_TYPE_LEN, "C_TYPE", NULL},
    {ft_set, 12, "AS SAC", NULL},
    // {ft_set, 3, "SC", NULL},
    {ft_set, 5, "PID", NULL},
    {ft_set, 16, "BITMAP", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_crc, CRC_8_SIZE, "CRC", NULL},
    {ft_end, 0, NULL, NULL},
};

struct_desc_t cc_ack_desc = {"DC_ACK_DESC", cc_ack_fields};
/**
 * DC ACK
 */
static field_desc dc_ack_fields[] = {
    {ft_set, D_TYPE_LEN, "D_TYP", NULL},
    // {ft_set, 3, "SC", NULL},
    {ft_set, 5, "PID", NULL},
    {ft_set, 16, "BITMAP", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_end, 0, NULL, NULL},
};

struct_desc_t dc_ack_desc = {"DC_ACK_DESC", dc_ack_fields};

/**
 * CC FRAG ACK
 */
static field_desc cc_frag_ack_fields[] = {
    {ft_set, C_TYPE_LEN, "C_TYPE", NULL},
    {ft_set, 12, "AS SAC", NULL},
    // {ft_set, 3, "SC", NULL},
    {ft_set, 5, "PID", NULL},
    {ft_set, 11, "SEQ1", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_crc, CRC_8_SIZE, "CRC", NULL},
    {ft_end, 0, NULL, NULL},
};

struct_desc_t cc_frag_ack_desc = {"DC_ACK_DESC", cc_frag_ack_fields};
/**
 * DC FRAG ACK
 */
static field_desc dc_frag_ack_fields[] = {
    {ft_set, D_TYPE_LEN, "D_TYP", NULL},
    // {ft_set, 3, "SC", NULL},
    {ft_set, 5, "PID", NULL},
    {ft_set, 11, "SEQ1", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_end, 0, NULL, NULL},
};

struct_desc_t dc_frag_ack_desc = {"DC_FRAG_ACK_DESC", dc_frag_ack_fields};

static field_desc dls_direct_fields[] = {
    {ft_set, 12, "AS SAC", NULL},
    {ft_set, 12, "GS SAC", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_dl_str, 0, "DATA", NULL},
    {ft_end, 0, NULL, NULL},
};
struct_desc_t dls_direct_desc = {"DLS_PDU", dls_direct_fields};
