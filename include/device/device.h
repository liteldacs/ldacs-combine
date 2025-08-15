//
// Created by 邹嘉旭 on 2023/12/20.
//

#ifndef LDACS_SIM_DEVICE_H
#define LDACS_SIM_DEVICE_H


#include <ldacs_sim.h>
#include <ldacs_utils.h>

#define REF_FREQ 960.0
#define CHANNEL_MAX 408
#define INVALID_FREQ -1.0

/**
 * TODO：未来需要做多信道监听时，把ld_dev_entity_s的send_th和recv_th用ld_thread_node[数组]进行替换
 */
typedef struct ld_dev_th_node_s {
    pthread_t th;

    void *(*func)(void *);

    int channel_num;
} ld_dev_th_node_t;

typedef struct ld_dev_entity_s {
    const char *name;
    lfqueue_t *msg_queue;
    pthread_t send_th;
    pthread_t recv_th;
    void *dev_para;

    bool freq_table[CHANNEL_MAX];

    l_err (*send_pkt)(void *, buffer_t *, ld_orient);

    void *(*recv_pkt)(void *);

    l_err (*set_freq)(void *, int, ld_orient);
} ld_dev_entity_t;

typedef struct ld_recv_args_s {
    ld_dev_entity_t *dev_en;

    void (*process_pkt)(void *);
} ld_recv_args_t;

l_err set_device(const char *dev_name, ld_dev_entity_t *dev_en);

void *start_recv(void *args);

/**
 * Determine if the freq is idle, if not, assign a new idle freq.
 * @param new_f new frequency assigned
 * @return real new freq
 */
double set_new_freq(ld_dev_entity_t *dev_en, double new_f, ld_orient ori);


#include "device_udp.h"


#endif //LDACS_SIM_DEVICE_H
