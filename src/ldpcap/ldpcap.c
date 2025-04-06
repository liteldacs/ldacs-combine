//
// Created by 邹嘉旭 on 2024/5/26.
//

#include "ldpcap.h"
#include "layer_core.h"

static ldpcap_t global_pcap = {
    .status = PCAP_CLOSE,
};

//设置一全局queue用来存储node

static void ldpcap_get_pkt(ld_queue_node_t *node) {
    channel_data_t *cdata = dup_channel_data(node->n_data);
    if (memrchr(cdata->buf, 0x00) == UNFOUND) return;
    lfqueue_put(global_pcap.pkt_queue, init_queue_node(E_TYP_ANY, cdata, free_channel_data));
}

l_err ldpcap_open() {
    if (global_pcap.status == PCAP_OPEN) return LD_ERR_INVALID;
    do {
        if (mac_register_interv(ldpcap_get_pkt) != LD_OK) break;
        global_pcap.pkt_queue = lfqueue_init();

        global_pcap.status = PCAP_OPEN;
        return LD_OK;
    } while (0);

    return LD_ERR_INTERNAL;
}

static void *ldpcap_looping_internel(void *args) {
    ld_queue_node_t *node;
    while (stop_flag == FALSE) {
        lfqueue_get_wait(global_pcap.pkt_queue, (void **) &node);
        channel_data_t *cdata = node->n_data;
        // log_buf(LOG_INFO, "LOOP", cdata->buf->ptr, cdata->buf->len);
    }
    return NULL;
}

l_err ldpcap_loop() {
    if (pthread_create(&global_pcap.th, NULL, ldpcap_looping_internel, NULL) != 0) {
        return LD_ERR_THREAD;
    }
    pthread_detach(global_pcap.th);
    return LD_OK;
}
