//
// Created by 邹嘉旭 on 2023/12/20.
//

#include "device.h"


l_err set_device(const char *dev_name, ld_dev_entity_t *dev_en) {
    size_t name_len = sizeof(dev_name);

    dev_en->name = dev_name;
    dev_en->msg_queue = lfqueue_init();

    memset(dev_en->freq_table, 0, CHANNEL_MAX);

    if (!strncmp(dev_name, "UDP", name_len)) {
        set_udp_device(dev_en);
    } else if (!strncmp(dev_name, "USRP", name_len)) {
        log_warn("USRP has not been implied");
        return LD_ERR_WRONG_PARA;
    } else {
        return LD_ERR_WRONG_PARA;
    }

    if (!set_new_freq(dev_en, config.init_fl_freq, FL)) {
        return LD_ERR_INTERNAL;
    }
    if (!set_new_freq(dev_en, config.init_rl_freq, RL)) {
        return LD_ERR_INTERNAL;
    }

    return LD_OK;
}

void *start_recv(void *args) {
    ld_recv_args_t *recv_args = args;
    ld_dev_entity_t *dev_en = recv_args->dev_en;

    if (pthread_create(&dev_en->recv_th, NULL, dev_en->recv_pkt, NULL) != 0) {
        pthread_exit(NULL);
    }
    pthread_detach(dev_en->recv_th);

    while (stop_flag == FALSE) {
        buffer_t *buf;
        if (lfqueue_get_wait(dev_en->msg_queue, (void **) &buf) == LD_OK) {
            recv_args->process_pkt(buf);
        }
    }
    return LD_OK;
}


double set_new_freq(ld_dev_entity_t *dev_en, double new_f, ld_orient ori) {
    if (!dev_en) {
        return INVALID_FREQ;
    }

    if (new_f < REF_FREQ) {
        return INVALID_FREQ;
    }

    int new_channel = (int) ((new_f - REF_FREQ) * 2);

    // Check if channel is out of range
    if (new_channel < 0 || new_channel >= CHANNEL_MAX) {
        return INVALID_FREQ;
    }

    // Find the next available channel if the current one is occupied
    if (dev_en->freq_table[new_channel]) {
        for (int i = new_channel + 1; i < CHANNEL_MAX; i++) {
            if (!dev_en->freq_table[i]) {
                new_channel = i;
                break;
            }
        }
        // If no available channel is found, return failure
        if (dev_en->freq_table[new_channel]) {
            return INVALID_FREQ;
        }
    }

    // Mark the channel as occupied
    dev_en->freq_table[new_channel] = 1;

    // Set the frequency
    if (dev_en->set_freq(new_channel, ori) != LD_OK) {
        return INVALID_FREQ; // Failed to set frequency
    }

    // Return the actual frequency
    return REF_FREQ + ((double) new_channel / 2);
}
