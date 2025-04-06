//
// Created by 邹嘉旭 on 2023/12/13.
//
#include "layer_core.h"

// ld_globaltimer_t g_timer;

ld_gtimer_t gtimer = {{.it_interval = {0, SF_TIMER}, .it_value = {0, 0}}};

static l_err prim_lock(ld_prim_t *prim_p) {
    if (prim_p == NULL)
        return LD_ERR_INVALID;
    return ld_lock(&prim_p->mutex);
}

static l_err prim_unlock(ld_prim_t *prim_p) {
    if (prim_p == NULL)
        return LD_ERR_INVALID;
    return ld_unlock(&prim_p->mutex);
}


