//
// Created by jiaxv on 25-10-3.
//

#ifndef SERVICE_H
#define SERVICE_H
#include "ldacs_sim.h"
#include <ld_buffer.h>

buffer_t *gen_ipv6_pkt(size_t len);

void generate_random_string(char *str, size_t length);

void send_singal_data();

void send_multi_datas();

void *send_user_data_func(void *args);

#endif //SERVICE_H
