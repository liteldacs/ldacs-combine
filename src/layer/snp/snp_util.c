//
// Created by 邹嘉旭 on 2024/5/13.
//
#include "ldacs_snp.h"

fsm_event_t snp_fsm_events[] = {
    {"SNP_CLOSED", NULL, NULL},
    {"SNP_AUTH", NULL, NULL},
    {"SNP_OPEN", NULL, NULL},
};

static struct sm_state_s snp_states[] = {
    {
        .data = "SNP_CLOSED",
        .entryAction = &sm_default_entry_action,
        .exitAction = &sm_default_exit_action,
        .transitions = (sm_transition_t[]){
            {SNP_EV_DEFAULT, (void *) "SNP_AUTH", &default_guard, NULL, &snp_states[SNP_AUTH]},
        },
        .numTransitions = 1,
    },
    {
        .data = "SNP_AUTH",
        .entryAction = &sm_default_entry_action,
        .exitAction = &sm_default_exit_action,
        .transitions = (sm_transition_t[]){
            {SNP_EV_DEFAULT, (void *) "SNP_CLOSED", &default_guard, NULL, &snp_states[SNP_CLOSED]},
            {SNP_EV_DEFAULT, (void *) "SNP_OPEN", &default_guard, NULL, &snp_states[SNP_OPEN]},
        },
        .numTransitions = 2,
    },
    {
        .data = "SNP_OPEN",
        .entryAction = &sm_default_entry_action,
        .exitAction = &sm_default_exit_action,
        .transitions = (sm_transition_t[]){
            {SNP_EV_DEFAULT, (void *) "SNP_CLOSED", &default_guard, NULL, &snp_states[SNP_CLOSED]},
            {SNP_EV_DEFAULT, (void *) "SNP_AUTH", &default_guard, NULL, &snp_states[SNP_AUTH]},
        },
        .numTransitions = 2,
    },
};

void init_snp_fsm(snp_layer_objs_t *snp_obj, enum SNP_FSM_STATES_E init_state) {
    stateM_init(&snp_obj->snp_fsm, &snp_states[init_state], NULL);
}
