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