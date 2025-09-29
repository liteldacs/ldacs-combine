//
// Created by jiaxv on 25-9-27.
//

#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <layer_rcu.h>

#define BACKEND_IP "127.0.0.1"
#define BACKEND_PORT 9876

typedef enum {
    REGISTER_AS,
    UPDATE_COORDINATE,
    START_STOP_AS,
    REGISTER_GS,
    SWITCH_AS,

    STOP_AS = 0xFF,
}DASHBOARD_FUNCTION;

typedef struct dashboard_func_define_s{
    DASHBOARD_FUNCTION func_e;
    json_tmpl_desc_t *tmpl;
}dashboard_func_define_t;

#pragma pack(1)
typedef struct dashboard_data_s {
    uint8_t type;
    char *data;
} dashboard_data_t;

typedef struct dashboard_update_coordinate_s {
    uint32_t UA;
    double longitude;
    double latitude;
}dashboard_update_coordinate_t;

typedef struct dashboard_register_gs_s {
    uint16_t TAG;
    double longitude;
    double latitude;
}dashboard_register_gs_t;
#pragma pack()

extern json_tmpl_desc_t dashboard_data_tmpl_desc;
extern json_tmpl_desc_t dashboard_update_coordinate_tmpl_desc;
extern json_tmpl_desc_t dashboard_register_gs_tmpl_desc;

extern const dashboard_func_define_t dashboard_func_defines[];


extern ld_service_t dashboard_service;

#endif //DASHBOARD_H
