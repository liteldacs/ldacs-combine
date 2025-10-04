//
// Created by jiaxv on 25-9-27.
//
#include "dashboard.h"

static json_tmpl_t dashboard_data_tmpl[] = {
    {cJSON_Number, sizeof(uint8_t), "type", "function", NULL},
    {cJSON_Raw, sizeof(char *), "data", "data", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

static json_tmpl_t dashboard_update_coordinate_tmpl[] = {
    {cJSON_Number, sizeof(uint32_t), "UA", "UA", NULL},
    {cJSON_Number, sizeof(double), "longitude", "longitude", &isfloat},
    {cJSON_Number, sizeof(double), "latitude", "latitude", &isfloat},
    {cJSON_Number, sizeof(uint8_t), "isDirect", "isDirect", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

static json_tmpl_t dashboard_register_gs_tmpl[] = {
    {cJSON_Number, sizeof(uint16_t), "tag", "tag", NULL},
    {cJSON_Number, sizeof(double), "longitude", "longitude", &isfloat},
    {cJSON_Number, sizeof(double), "latitude", "latitude", &isfloat},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

static json_tmpl_t dashboard_switch_as_tmpl[] = {
    {cJSON_Number, sizeof(uint32_t), "UA", "UA", NULL},
    {cJSON_Number, sizeof(uint16_t), "gst", "gst", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

static json_tmpl_t dashboard_received_msg_tmpl[] = {
    {cJSON_Number, sizeof(uint8_t), "orient", "orient", NULL},
    {cJSON_Number, sizeof(uint8_t), "type", "type", NULL},
    {cJSON_Number, sizeof(uint32_t), "sender", "sender", NULL},
    {cJSON_Number, sizeof(uint32_t), "receiver", "receiver", NULL},
    {cJSON_String, sizeof(buffer_t *), "message", "message", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

static json_tmpl_t dashboard_as_exit_tmpl[] = {
    {cJSON_Number, sizeof(uint32_t), "UA", "UA", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

json_tmpl_desc_t dashboard_data_tmpl_desc = {
    .desc = "DASHBOARD_DATA",
    .tmpl = dashboard_data_tmpl,
    .size = sizeof(dashboard_data_t)
};

json_tmpl_desc_t dashboard_update_coordinate_tmpl_desc = {
    .desc = "DASHBOARD_UPDATE_COORDINATE",
    .tmpl = dashboard_update_coordinate_tmpl,
    .size = sizeof(dashboard_update_coordinate_t)
};

json_tmpl_desc_t dashboard_register_gs_tmpl_desc = {
    .desc = "DASHBOARD_REGISTER_GS",
    .tmpl = dashboard_register_gs_tmpl,
    .size = sizeof(dashboard_register_gs_t)
};

json_tmpl_desc_t dashboard_switch_as_tmpl_desc = {
    .desc = "DASHBOARD_SWITCH_AS",
    .tmpl = dashboard_switch_as_tmpl,
    .size = sizeof(dashboard_switch_as_t)
};

json_tmpl_desc_t dashboard_received_msg_tmpl_desc = {
    .desc = "DASHBOARD_RECEIVED_MSG",
    .tmpl = dashboard_received_msg_tmpl,
    .size = sizeof(dashboard_received_msg_t)
};

json_tmpl_desc_t dashboard_as_exit_tmpl_desc = {
    .desc = "DASHBOARD_EXIT_AS",
    .tmpl = dashboard_as_exit_tmpl,
    .size = sizeof(dashboard_as_exit_t)
};

const dashboard_func_define_t dashboard_func_defines[] = {
    {AS_REGISTER, &dashboard_update_coordinate_tmpl_desc},
    {AS_UPDATE_COORDINATE, &dashboard_update_coordinate_tmpl_desc},
    {DASHBOARD_START_STOP_AS, NULL},
    {GS_REGISTER, &dashboard_register_gs_tmpl_desc},
    {DASHBOARD_SWITCH_AS, &dashboard_switch_as_tmpl_desc},
    {AS_GS_RECEIVED_MSG, &dashboard_received_msg_tmpl_desc},
    {GS_ACCESS_AS, &as_info_upd_tmpl_desc},
    {GS_AS_EXITED, &dashboard_as_exit_tmpl_desc}
};