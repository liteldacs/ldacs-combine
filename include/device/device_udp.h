//
// Created by 邹嘉旭 on 2023/12/20.
//

#ifndef LDACS_SIM_DEVICE_UDP_H
#define LDACS_SIM_DEVICE_UDP_H

#include "device.h"

#define REF_PORT 10000

typedef struct ld_dev_udp_para_s {
    ld_dev_entity_t *dev;
    int rl_send_fd;
    int rl_recv_fd;
    int fl_send_fd;
    int fl_recv_fd;
    struct sockaddr_in fl_send_addr;
    struct sockaddr_in rl_send_addr;
} ld_dev_udp_para_t;

ld_dev_udp_para_t *set_udp_device(ld_dev_entity_t *en);

#endif //LDACS_SIM_DEVICE_UDP_H
