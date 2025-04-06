//
// Created by 邹嘉旭 on 2025/3/7.
//
//
// Created by 邹嘉旭 on 2024/3/21.
//
#include "ldacs_lme.h"


struct sm_state_s ld_authc_states[] = {
    {
        .data = "LD_AUTHC_A0",
        .entryAction = &sm_default_entry_action,
        .exitAction = &sm_default_exit_action,
        .transitions = (sm_transition_t[]){
            {
                LD_AUTHC_EV_DEFAULT, (void *) "LD_AUTHC_A1", &default_guard, NULL,
                &ld_authc_states[LD_AUTHC_A1]
            },
        },
        .numTransitions = 1,
    },
    {
        .data = "LD_AUTHC_A1",
        .entryAction = &sm_default_entry_action,
        .exitAction = &sm_default_exit_action,
        .transitions = (sm_transition_t[]){
            {LD_AUTHC_EV_DEFAULT, (void *) "LD_AUTHC_A2", &default_guard, NULL, &ld_authc_states[LD_AUTHC_A2]},
            {LD_AUTHC_EV_DEFAULT, (void *) "LD_AUTHC_A0", &default_guard, NULL, &ld_authc_states[LD_AUTHC_A0]},
        },
        .numTransitions = 2,
    },
    {
        .data = "LD_AUTHC_A2",
        .entryAction = &sm_default_entry_action,
        .exitAction = &sm_default_exit_action,
        .transitions = (sm_transition_t[]){
            {
                LD_AUTHC_EV_DEFAULT, (void *) "LD_AUTHC_A0", &default_guard, NULL,
                &ld_authc_states[LD_AUTHC_A0]
            },
        },
        .numTransitions = 1,
    },
    {
        .data = "LD_AUTHC_G0",
        .entryAction = &sm_default_entry_action,
        .exitAction = &sm_default_exit_action,
        .transitions = (sm_transition_t[]){
            {
                LD_AUTHC_EV_DEFAULT, (void *) "LD_AUTHC_G1", &default_guard, NULL,
                &ld_authc_states[LD_AUTHC_G1]
            },
        },
        .numTransitions = 1,
    },
    {
        .data = "LD_AUTHC_G1",
        .entryAction = &sm_default_entry_action,
        .exitAction = &sm_default_exit_action,
        .transitions = (sm_transition_t[]){
            {LD_AUTHC_EV_DEFAULT, (void *) "LD_AUTHC_G2", &default_guard, NULL, &ld_authc_states[LD_AUTHC_G2]},
            {LD_AUTHC_EV_DEFAULT, (void *) "LD_AUTHC_G0", &default_guard, NULL, &ld_authc_states[LD_AUTHC_G0]},
        },
        .numTransitions = 2,
    },
    {
        .data = "LD_AUTHC_G2",
        .entryAction = &sm_default_entry_action,
        .exitAction = &sm_default_exit_action,
        .transitions = (sm_transition_t[]){
            {
                LD_AUTHC_EV_DEFAULT, (void *) "LD_AUTHC_G0", &default_guard, NULL,
                &ld_authc_states[LD_AUTHC_G0]
            },
        },
        .numTransitions = 1,
    },
};

buffer_t *get_auc_sharedinfo_buf(auc_sharedinfo_t *info) {
    buffer_t *info_buf = init_buffer_unptr();
    pb_stream pbs;
    zero(&pbs);
    uint8_t res_str[32] = {0};

    init_pbs(&pbs, res_str, 32, "SHAREDINFO");
    if (!out_struct(info, &auc_sharedinfo_desc, &pbs, NULL)) {
        free_buffer(info_buf);
        return NULL;
    }
    close_output_pbs(&pbs);
    CLONE_TO_CHUNK(*info_buf, pbs.start, pbs_offset(&pbs));
    return info_buf;
}

l_err generate_auc_kdf(buffer_t *random, KEY_HANDLE*key_as_sgw, KEY_HANDLE*key_as_gs,
                       buffer_t **key_as_gs_raw) {
    UA_STR(ua_as);
    UA_STR(ua_gs);
    UA_STR(ua_sgw);
    get_ua_str(10010, ua_as);
    get_ua_str(10086, ua_gs);
    get_ua_str(10000, ua_sgw);

    switch (config.role) {
        case LD_AS:
            as_derive_keys(random->ptr, random->len, ua_as, ua_gs, ua_sgw, key_as_sgw, key_as_gs);
        // log_buf(LOG_ERROR, "AS KEY", (*(buffer_t **)key_as_gs)->ptr, (*(buffer_t **)key_as_gs)->len);
            break;
        case LD_SGW:
            sgw_derive_keys(random->ptr, random->len, ua_as, ua_gs, ua_sgw, key_as_sgw, key_as_gs_raw);

        // log_buf(LOG_ERROR, "SGW KEY", (*(buffer_t **)key_as_gs_raw)->ptr, (*(buffer_t **)key_as_gs_raw)->len);
            break;
    }
    return LD_OK;
}
