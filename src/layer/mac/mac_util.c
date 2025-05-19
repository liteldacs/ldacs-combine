//
// Created by 邹嘉旭 on 2024/3/26.
//
#include "ldacs_mac.h"

fsm_event_t mac_fsm_events[] = {
    {"MAC_FSCANNING", NULL, NULL},
    {"MAC_CSCANNING", NULL, NULL},
    {"MAC_CONNECTING", NULL, NULL},
    {"MAC_AUTH", NULL, NULL},
    {"MAC_OPEN", NULL, NULL},
    {"MAC_HO2", entry_MAC_HO2, NULL},
};

static struct sm_state_s mac_states[] = {
    {
        .data = "MAC_FSCANNING",
        .entryAction = &sm_default_entry_action,
        .exitAction = &sm_default_exit_action,
        .transitions = (sm_transition_t[]){
            {MAC_EV_DEFAULT, (void *) "MAC_CSCANNING", &default_guard, NULL, &mac_states[MAC_CSCANNING]},
        },
        .numTransitions = 1,
    },
    {
        .data = "MAC_CSCANNING",
        .entryAction = &sm_default_entry_action,
        .exitAction = &sm_default_exit_action,
        .transitions = (sm_transition_t[]){
            {MAC_EV_DEFAULT, (void *) "MAC_FSCANNING", &default_guard, NULL, &mac_states[MAC_FSCANNING]},
            {MAC_EV_DEFAULT, (void *) "MAC_CONNECTING", &default_guard, NULL, &mac_states[MAC_CONNECTING]},
        },
        .numTransitions = 2,
    },
    {
        .data = "MAC_CONNECTING",
        .entryAction = &sm_default_entry_action,
        .exitAction = &sm_default_exit_action,
        .transitions = (sm_transition_t[]){
            {MAC_EV_DEFAULT, (void *) "MAC_CSCANING", &default_guard, NULL, &mac_states[MAC_CSCANNING]},
            {MAC_EV_DEFAULT, (void *) "MAC_AUTH", &default_guard, NULL, &mac_states[MAC_AUTH]},
        },
        .numTransitions = 2,
    },
    {
        .data = "MAC_AUTH",
        .entryAction = &sm_default_entry_action,
        .exitAction = &sm_default_exit_action,
        .transitions = (sm_transition_t[]){
            {MAC_EV_DEFAULT, (void *) "MAC_CONNECTING", &default_guard, NULL, &mac_states[MAC_CONNECTING]},
            {MAC_EV_DEFAULT, (void *) "MAC_OPEN", &default_guard, NULL, &mac_states[MAC_OPEN]},
            {MAC_EV_DEFAULT, (void *) "MAC_FSCANNING", &default_guard, NULL, &mac_states[MAC_FSCANNING]},
        },
        .numTransitions = 3,
    },
    {
        .data = "MAC_OPEN",
        .entryAction = &sm_default_entry_action,
        .exitAction = &sm_default_exit_action,
        .transitions = (sm_transition_t[]){
            {MAC_EV_DEFAULT, (void *) "MAC_AUTH", &default_guard, NULL, &mac_states[MAC_AUTH]},
            {MAC_EV_DEFAULT, (void *) "MAC_HO2", &default_guard, NULL, &mac_states[MAC_HO2]},
            {MAC_EV_DEFAULT, (void *) "MAC_FSCANNING", &default_guard, NULL, &mac_states[MAC_FSCANNING]},
        },
        .numTransitions = 3,
    },
    {
        .data = "MAC_HO2",
        .entryAction = &sm_default_entry_action,
        .exitAction = &sm_default_exit_action,
        .transitions = (sm_transition_t[]){
            {MAC_EV_DEFAULT, (void *) "MAC_OPEN", &default_guard, NULL, &mac_states[MAC_OPEN]},
        },
        .numTransitions = 1,
    },
};

void init_mac_fsm(mac_layer_objs_t *mac_obj, enum MAC_FSM_STATES_E init_state) {
    stateM_init(&mac_obj->mac_fsm, &mac_states[init_state], NULL);
}

l_err mac_register_interv(inter_func func) {
    mac_interv_t *interv = init_mac_interv(func);
    list_add_tail(&interv->lpointer, mac_layer_objs.interv_head);

    return LD_OK;
}

channel_data_t *init_channel_data(enum CHANNEL_E channel, enum ELE_TYP type, uint16_t SAC) {
    channel_data_t *data = malloc(sizeof(channel_data_t));
    data->channel = channel;
    data->type = type;
    data->SAC = SAC;
    data->buf = init_buffer_unptr();
    return data;
}

void free_channel_data(void *rdata) {
    channel_data_t *rl_data = rdata;
    if (rl_data->buf) free_buffer(rl_data->buf);
    free(rl_data);
}


channel_data_t *dup_channel_data(channel_data_t *src) {
    channel_data_t *cdata = calloc(1, sizeof(channel_data_t));
    memcpy(cdata, src, sizeof(channel_data_t));
    cdata->buf = init_buffer_unptr();
    CLONE_TO_CHUNK(*cdata->buf, src->buf->ptr, src->buf->len);
    return cdata;
}


ld_rpso_t *init_rpsos(uint8_t start, size_t size) {
    ld_rpso_t *mac_rpso_sac = calloc(1, sizeof(ld_rpso_t));
    mac_rpso_sac->avail_start = start;
    mac_rpso_sac->avail_sz = size;
    return mac_rpso_sac;
}

void *delay_put_rpso(void *args) {
    usleep(MF_TIMER / 1000);
    // if (((ld_rpso_t *) args)->avail_sz == 154) {
    //     log_warn("PUT RPSO!!!");
    // }
    lfqueue_put(mac_layer_objs.rpso_queue, args);
    return NULL;
}

void set_rpsos(ld_rpso_t *rpsos) {
    // 未来应改成multitimer
    pthread_create(&mac_layer_objs.rpso_th, NULL, delay_put_rpso, rpsos);
    pthread_detach(mac_layer_objs.rpso_th);
}

l_err set_rl_param(const uint8_t RPSO, const uint8_t NRPS) {
    ld_lock(&mac_layer_objs.RPSO.mutex);
    mac_layer_objs.RPSO.value = RPSO;
    ld_unlock(&mac_layer_objs.RPSO.mutex);

    ld_lock(&mac_layer_objs.NRPS.mutex);
    mac_layer_objs.NRPS.value = NRPS;
    ld_unlock(&mac_layer_objs.NRPS.mutex);

    return LD_OK;
}
