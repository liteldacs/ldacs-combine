//
// Created by 邹嘉旭 on 2024/8/19.
//
#include "ldacs_phy.h"


static json_tmpl_t phy_json_hdr_j_tmpl[] = {
    {cJSON_Number, sizeof(uint8_t), "channel", "channel", NULL},
    // {cJSON_Number, sizeof(uint8_t), "sf_seq", "sf_seq", NULL},
    {cJSON_Number, sizeof(uint8_t), "strength", "strength", NULL},
    {cJSON_Raw, sizeof(char *), "raw", "raw", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

static json_tmpl_t bcch_j_tmpl[] = {
    {cJSON_String, sizeof(buffer_t *), "BC_1", "BC_1", NULL},
    {cJSON_String, sizeof(buffer_t *), "BC_2", "BC_2", NULL},
    {cJSON_String, sizeof(buffer_t *), "BC_3", "BC_3", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

static json_tmpl_t cc_dch_j_tmpl[] = {
    {cJSON_String, sizeof(buffer_t *), "1-6", "1-6", NULL},
    {cJSON_String, sizeof(buffer_t *), "7-12", "7-12", NULL},
    {cJSON_String, sizeof(buffer_t *), "13-21", "13-21", NULL},
    {cJSON_String, sizeof(buffer_t *), "22-27", "22-27", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

static json_tmpl_t rach_j_tmpl[] = {
    {cJSON_String, sizeof(buffer_t *), "1", "1", NULL},
    {cJSON_String, sizeof(buffer_t *), "2", "2", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

static json_tmpl_t dcch_j_tmpl[] = {
    {cJSON_Number, sizeof(uint8_t), "sync-tile", "sync-tile", NULL},
    {cJSON_Number, sizeof(uint16_t), "slot", "slot", NULL},
    {cJSON_String, sizeof(buffer_t *), "dcch", "dcch", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

static json_tmpl_t rl_dch_j_tmpl[] = {
    {cJSON_Number, sizeof(uint8_t), "start_slot", "start_slot", NULL},
    {cJSON_Number, sizeof(uint8_t), "stop_slot", "stop_slot", NULL},
    {cJSON_String, sizeof(buffer_t *), "rl_dch", "rl_dch", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

json_tmpl_desc_t bcch_j_tmpl_desc = {"BCCH", bcch_j_tmpl, sizeof(bcch_pdu_t)};
json_tmpl_desc_t cc_dch_j_tmpl_desc = {"CC_DCH", cc_dch_j_tmpl, sizeof(cc_dch_pdu_t)};
json_tmpl_desc_t rach_j_tmpl_desc = {"RACH", rach_j_tmpl, sizeof(rach_pdu_t)};
json_tmpl_desc_t dcch_j_tmpl_desc = {"DCCH", dcch_j_tmpl, sizeof(dcch_pdu_t)};
json_tmpl_desc_t rl_dch_j_tmpl_desc = {"RL_DCH", rl_dch_j_tmpl, sizeof(rl_dch_pdu_t)};
json_tmpl_desc_t phy_json_hdr_j_tmpl_desc = {"JSON_HDR", phy_json_hdr_j_tmpl, sizeof(phy_json_hdr_t)};

