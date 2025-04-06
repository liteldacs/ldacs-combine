//
// Created by 邹嘉旭 on 2024/3/22.
//

#include "ldacs_lme.h"
#include "key.h"

fsm_event_t lme_fsm_events[] = {
    {"LME_FSCANNING", NULL, NULL},
    {"LME_CSCANNING", entry_LME_CSCANNING, NULL},
    {"LME_CONNECTING", entry_LME_CONNECTING, NULL},
    {"LME_AUTH", entry_LME_AUTH, NULL},
    {"LME_OPEN", entry_LME_OPEN, NULL},
};

static struct sm_state_s lme_states[] = {
    {
        .data = "LME_FSCANNING",
        .entryAction = &sm_default_entry_action,
        .exitAction = &sm_default_exit_action,
        .transitions = (sm_transition_t[]){
            {LME_EV_DEFAULT, (void *) "LME_CSCANNING", &default_guard, NULL, &lme_states[LME_CSCANNING]},
        },
        .numTransitions = 1,
    },
    {
        .data = "LME_CSCANNING",
        .entryAction = &sm_default_entry_action,
        .exitAction = &sm_default_exit_action,
        .transitions = (sm_transition_t[]){
            {LME_EV_DEFAULT, (void *) "LME_CONNECTING", &default_guard, NULL, &lme_states[LME_CONNECTING]},
            {LME_EV_DEFAULT, (void *) "LME_FSCANNING", &default_guard, NULL, &lme_states[LME_FSCANNING]},
        },
        .numTransitions = 2,
    },
    {
        .data = "LME_CONNECTING",
        .entryAction = &sm_default_entry_action,
        .exitAction = &sm_default_exit_action,
        .transitions = (sm_transition_t[]){
            {LME_EV_DEFAULT, (void *) "LME_AUTH", &default_guard, NULL, &lme_states[LME_AUTH]},
            {LME_EV_DEFAULT, (void *) "LME_CSCANNING", &default_guard, NULL, &lme_states[LME_CSCANNING]},
        },
        .numTransitions = 2,
    },
    {
        .data = "LME_AUTH",
        .entryAction = &sm_default_entry_action,
        .exitAction = &sm_default_exit_action,
        .transitions = (sm_transition_t[]){
            {LME_EV_DEFAULT, (void *) "LME_OPEN", &default_guard, NULL, &lme_states[LME_OPEN]},
            {
                LME_EV_DEFAULT, (void *) "LME_FSCANNING", default_guard, exit_LME_CONN_OPEN_action,
                &lme_states[LME_FSCANNING]
            },
            {LME_EV_DEFAULT, (void *) "LME_CONNECTING", &default_guard, NULL, &lme_states[LME_CONNECTING]},
        },
        .numTransitions = 3,
    },
    {
        .data = "LME_OPEN",
        .entryAction = &sm_default_entry_action,
        .exitAction = &sm_default_exit_action,
        .transitions = (sm_transition_t[]){
            {
                LME_EV_DEFAULT, (void *) "LME_FSCANNING", default_guard, exit_LME_CONN_OPEN_action,
                &lme_states[LME_FSCANNING]
            },
            {LME_EV_DEFAULT, (void *) "LME_CONNECTING", &default_guard, NULL, &lme_states[LME_CONNECTING]},
        },
        .numTransitions = 2,
    }
};

void init_lme_fsm(lme_layer_objs_t *lme_obj, enum LME_FSM_STATES_E init_state) {
    stateM_init(&lme_obj->lme_fsm, &lme_states[init_state], NULL);
}

void *lme_iter_ua_func(void *item, void *judge) {
    const lme_as_man_t *as_man = item;
    uint32_t *target_UA = judge;
    if (as_man->AS_UA == *target_UA)
        return item;
    return NULL;
}

void *map_iter(void *(*iter_func)(void *, void *), void *judge) {
    size_t iter = 0;
    void *item;
    void *ret;
    while (hashmap_iter(lme_layer_objs.LME_GS_AUTH_AS, &iter, &item)) {
        if ((ret = iter_func(item, judge)) == NULL)
            continue;
        return ret;
    }
    return NULL;
}


lme_as_man_t *init_as_man(uint16_t AS_SAC, uint32_t AS_UA, uint16_t AS_CURR_GS_SAC,
                          enum LD_AUTHC_STATES_E init_st) {
    lme_as_man_t *as_man = calloc(1, sizeof(lme_as_man_t));

    as_man->AS_SAC = AS_SAC;
    as_man->AS_UA = AS_UA;
    as_man->AS_CURR_GS_SAC = AS_CURR_GS_SAC;

    as_man->AUTHC_MACLEN = AUTHC_MACLEN_256; /* default mac len is 256  */
    as_man->AUTHC_AUTH_ID = AUTHC_AUTH_SM3HMAC;
    as_man->AUTHC_ENC_ID = AUTHC_ENC_SM4_CBC;
    as_man->AUTHC_KLEN = AUTHC_KLEN_128;

    // as_man->CO = DEFAULT_CO;
    // as_man->CO.mutex = PTHREAD_MUTEX_INITIALIZER;
    // as_man->CO.co_n = 2;
    // as_man->CO.co = {1, 15};
    zero(&as_man->CO);
    as_man->RPSO = DEFAULT_RPSO;
    as_man->NRPS = DEFAULT_NRPS;

    as_man->send_T_SQN = as_man->recv_T_SQN = 0;

    as_man->shared_random = NULL;
    as_man->key_as_gs_b = NULL;

    UA_STR(ua_as);
    UA_STR(ua_sgw);
    if (config.role == LD_AS) {
        key_get_handle(config.role, get_ua_str(config.UA, ua_as), get_ua_str(10000, ua_sgw), ROOT_KEY,
                       &as_man->key_as_sgw_r_h);
    } else if (config.role == LD_SGW) {
        as_man->key_as_gs_b = init_buffer_unptr();
        key_get_handle(config.role, get_ua_str(10010, ua_as), get_ua_str(10000, ua_sgw), ROOT_KEY,
                       &as_man->key_as_sgw_r_h);
    }


    stateM_init(&as_man->auth_fsm, &ld_authc_states[init_st], NULL);

    if (config.role == LD_AS || config.role == LD_GS)
        as_man_update_handler(as_man);

    as_man->gs_conn = NULL;
    as_man->gs_finish_auth = FALSE;
    as_man->gsnf_count = 0;

    return as_man;
}

l_err clear_as_man(lme_as_man_t *as_man) {
    uint16_t CO;
    if (clear_mac_CO(as_man->AS_SAC, &CO) != LD_ERR_WRONG_PARA) {
        free_CO(CO);
    }
    if (as_man->shared_random) free_buffer(as_man->shared_random);
    if (as_man->key_as_gs_b) free_buffer(as_man->key_as_gs_b);
    // as_man->AS_SAC = DEFAULT_SAC;
    // zero(&as_man->CO);
    return LD_OK;
}

l_err free_as_man(lme_as_man_t *as_man) {
    if (as_man) {
        clear_as_man(as_man);
        free(as_man);
        return LD_OK;
    }
    return LD_ERR_NULL;
}

l_err as_man_update_handler(lme_as_man_t *as_man) {
    l_err err;
    if ((err = preempt_prim(&LME_STATE_IND_PRIM, LME_AS_UPDATE, as_man, NULL, 0, 0))) {
        return err;
    }
    return LD_OK;
}

l_err as_man_update_key_handler(lme_as_man_t *as_man, void *key, uint64_t value, size_t sz, const char *key_str) {
    l_err err;
    switch (sz) {
        case 1:
            *(uint8_t *) key = value;
            break;
        case 2:
            *(uint16_t *) key = value;
            break;
        case 3:
            *(uint32_t *) key = value;
            break;
        case 4:
            *(uint64_t *) key = value;
            break;
        default:
            //TODO: 错误处理
            break;
    }

    if (config.role == LD_AS || config.role == LD_GS) {
        buffer_t *buf = init_buffer_unptr();
        CLONE_TO_CHUNK(*buf, key_str, strlen(key_str));

        if ((err = preempt_prim(&LME_STATE_IND_PRIM, LME_AS_KEY_UPDATE, &(as_info_key_upd_t){
                                    .ua = as_man->AS_UA,
                                    .key = buf,
                                    .value = value,
                                }, NULL, 0, 0))) {
            return err;
        }
        free_buffer(buf);
    }
    return LD_OK;
}


l_err add_co(ld_co_t *ld_co, uint16_t new_CO) {
    ld_lock(&ld_co->mutex);
    ld_co->co[ld_co->co_n++] = new_CO;
    ld_unlock(&ld_co->mutex);
    return LD_OK;
}



