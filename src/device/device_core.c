//
// Created by 邹嘉旭 on 2023/12/20.
//

#include "device.h"


device_entity_t *set_device(const char *dev_name, void (*process_func)(void *)) {
    size_t name_len = sizeof(dev_name);

    device_entity_t *dev_en = NULL;
    if (!strncmp(dev_name, "UDP", name_len)) {
        dev_en = (device_entity_t *)setup_udp_device(process_func);
    } else if (!strncmp(dev_name, "USRP", name_len)) {
        log_warn("USRP has not been implied");
        return NULL;
    } else {
        return NULL;
    }
    if (!dev_en) {
        log_error("Cannot initialize device");
        return NULL;
    }

    return dev_en;
}

void *start_recv(void *args) {
    device_entity_t *dev_en = args;
    if (!dev_en)    return NULL;

    if (pthread_create(&dev_en->recv_th, NULL, dev_en->recv_pkt, dev_en) != 0) {
        pthread_exit(NULL);
    }
    pthread_detach(dev_en->recv_th);

    while (stop_flag == FALSE) {
        buffer_t *buf;
        if (lfqueue_get_wait(dev_en->msg_queue, (void **) &buf) == LD_OK) {
            if (dev_en->process_func) {
                dev_en->process_func(buf);
            }
        }
    }
    return LD_OK;
}


double set_new_freq(device_entity_t *dev_en, double new_f, ld_orient ori) {
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
    if (dev_en->set_freq(dev_en, new_channel, ori) != LD_OK) {
        return INVALID_FREQ; // Failed to set frequency
    }

    // Return the actual frequency
    return REF_FREQ + ((double) new_channel / 2);
}
