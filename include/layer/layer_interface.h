//
// Created by 邹嘉旭 on 2024/4/20.
//

#ifndef LAYER_INTERFACE_H
#define LAYER_INTERFACE_H

#include "layer_core.h"
#include "layer_rcu.h"

typedef struct int_layer_obj_s {
    void (*msg_handler)(user_msg_t *);
} int_layer_obj_t;

l_err register_int_handler(void (*handler)(user_msg_t *));

void SN_SAPD_U_cb(ld_prim_t *prim);

l_err send_user_data_as(uint8_t *data, size_t sz);

#endif //LAYER_INTERFACE_H
