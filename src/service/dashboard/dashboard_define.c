//
// Created by jiaxv on 25-9-27.
//
#include "dashboard.h"

static json_tmpl_t dashboard_data_tmpl[] = {
    {cJSON_Number, sizeof(uint8_t), "func", "function", NULL},
    {cJSON_Raw, sizeof(char *), "data", "data", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

static json_tmpl_t dashboard_update_coordinate_tmpl[] = {
    {cJSON_Number, sizeof(uint32_t), "UA", "function", NULL},
    {cJSON_Number, sizeof(double), "longitude", "longitude", &isfloat},
    {cJSON_Number, sizeof(double), "latitude", "latitude", &isfloat},
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

const dashboard_func_define_t dashboard_func_defines[] = {
    {REGISTER_AS, &dashboard_update_coordinate_tmpl_desc},
    {UPDATA_COORDINATE, &dashboard_update_coordinate_tmpl_desc},
};