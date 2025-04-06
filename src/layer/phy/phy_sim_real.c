//
// Created by 邹嘉旭 on 2024/2/21.
//
#include "ldacs_phy.h"

static phy_layer_objs_t *phy_obj;

l_err init_sim_real(phy_layer_objs_t *obj_p) {
    phy_obj = obj_p;
}

l_err upward_real(void *data) {
    return LD_OK;
}

l_err downward_real(ld_prim_t *prim, buffer_t **in_bufs, buffer_t **out_buf) {
    return LD_OK;
}

