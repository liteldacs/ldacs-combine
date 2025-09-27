//
// Created by jiaxv on 25-9-27.
//
#include "dashboard.h"

static json_tmpl_t dashboard_data_tmpl[] = {
    {cJSON_Number, sizeof(uint8_t), "func", "function", NULL},
    {cJSON_Raw, sizeof(char *), "data", "data", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

json_tmpl_desc_t dashboard_data_tmpl_desc = {
    .desc = "DASHBOARD_DATA",
    .tmpl = dashboard_data_tmpl,
    .size = sizeof(dashboard_data_t)
};

const dashboard_func_define_t dashboard_func_defines[] = {
    {UPDATA_COORDINATE, NULL},
};