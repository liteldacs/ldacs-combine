//
// Created by 邹嘉旭 on 2024/5/28.
//

#ifndef LDPCAP_H
#define LDPCAP_H

#include <ldacs_sim.h>
#include <ldacs_utils.h>
#include "layer/ldacs_mac.h"

enum LDPCAP_STATUS_E {
    PCAP_OPEN,
    PCAP_CLOSE,
};

typedef struct ldpcap_s {
    enum LDPCAP_STATUS_E status;
    lfqueue_t *pkt_queue;
    pthread_t th;
} ldpcap_t;

typedef struct pcap_session_s {
    lfqueue_t *pkt_queue;
    pthread_t th;
} pcap_session_t;

l_err ldpcap_open();

l_err ldpcap_loop();
#endif //LDPCAP_H
