//
// Created by 邹嘉旭 on 2024/8/19.
//

#include "layer_rcu.h"

json_tmpl_t sse_state_tmpl[] = {
    {cJSON_Number, sizeof(uint32_t), "ua", "ua", NULL},
    {cJSON_Number, sizeof(uint16_t), "sac", "sac", NULL},
    {cJSON_Number, sizeof(uint16_t), "state", "state", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};


json_tmpl_t as_info_key_upd_tmpl[] = {
    {cJSON_Number, sizeof(uint32_t), "ua", "ua", NULL},
    {cJSON_String, sizeof(buffer_t *), "key", "key", NULL},
    {cJSON_Number, sizeof(uint64_t), "value", "value", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

json_tmpl_t as_info_upd_tmpl[] = {
    {cJSON_Number, sizeof(uint32_t), "UA", "UA", NULL},
    {cJSON_Number, sizeof(uint16_t), "AS_SAC", "AS_SAC", NULL},
    {cJSON_Number, sizeof(uint16_t), "GS_SAC", "GS_SAC", NULL},
    {cJSON_Number, sizeof(uint8_t), "maclen", "maclen", NULL},
    {cJSON_Number, sizeof(uint8_t), "authid", "authid", NULL},
    {cJSON_Number, sizeof(uint8_t), "encid", "encid", NULL},
    {cJSON_Number, sizeof(uint8_t), "kdflen", "kdflen", NULL},
    {cJSON_Number, sizeof(uint16_t), "CO", "CO", NULL},
    {cJSON_Number, sizeof(uint8_t), "RPSO", "RPSO", NULL},
    {cJSON_Number, sizeof(uint8_t), "NRPS", "NRPS", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};


json_tmpl_t user_msg_tmpl[] = {
    {cJSON_Number, sizeof(uint16_t), "AS_SAC", "sac", NULL},
    {cJSON_Number, sizeof(uint16_t), "GS_SAC", "sac", NULL},
    {cJSON_String, sizeof(buffer_t *), "msg", "msg", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};
