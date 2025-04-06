//
// Created by 邹嘉旭 on 2024/3/25.
//

#include "ldacs_dls.h"


void dls_enode_print_func(void *item) {
    const dls_entity_t *node = item;
    fprintf(stderr, "{%u}\n", node->AS_SAC);
}

fsm_event_t dls_fsm_events[] = {
    {"DLS_CLOSED", NULL, NULL},
    {"DLS_OPEN", NULL, NULL},
};

static struct sm_state_s dls_states[] = {
    {
        .data = "DLS_CLOSED",
        .entryAction = &sm_default_entry_action,
        .exitAction = &sm_default_exit_action,
        .transitions = (sm_transition_t[]){
            {DLS_EV_DEFAULT, (void *) "DLS_OPEN", &default_guard, NULL, &dls_states[DLS_OPEN]},
        },
        .numTransitions = 1,
    },
    {
        .data = "DLS_OPEN",
        .entryAction = &sm_default_entry_action,
        .exitAction = &sm_default_exit_action,
        .transitions = (sm_transition_t[]){
            {DLS_EV_DEFAULT, (void *) "DLS_CLOSED", &default_guard, NULL, &dls_states[DLS_CLOSED]},
        },
        .numTransitions = 1,
    },
};

void init_dls_fsm(dls_layer_objs_t *dls_obj, enum DLS_FSM_STATES_E init_state) {
    stateM_init(&dls_obj->dls_fsm, &dls_states[init_state], NULL);
}
