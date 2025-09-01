//
// Created by 邹嘉旭 on 2023/12/20.
//

#include "device/device_udp.h"

l_err init_udp_bd_send(ld_dev_udp_para_t *udp_para, int send_port) {
    int on = 1;

    int bd_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (bd_fd == ERROR) {
        log_error("Create broadcast socket failed !\n");
        return LD_ERR_INTERNAL;
    }

    if (setsockopt(bd_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == ERROR) {
        log_warn("setsockopt SO_REUSEADDR failed!\n");
        // 失败不返回，只是警告，因为不是所有平台都严格支持（如某些BSD）
    }

#ifdef SO_REUSEPORT
    if (setsockopt(bd_fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) == ERROR) {
        log_warn("setsockopt SO_REUSEPORT failed! (not critical)\n");
    }
#endif

    if (setsockopt(bd_fd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) == ERROR) {
        log_error("setsockopt failed!\n");
        return LD_ERR_INTERNAL;
    }

    struct sockaddr_in *addr = NULL;
    if (config.role == LD_AS) {
        addr = &udp_para->rl_send_addr;
        udp_para->rl_send_fd = bd_fd;
    }else {
        addr = &udp_para->fl_send_addr;
        udp_para->fl_send_fd = bd_fd;
    }

    bzero(addr, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(send_port);
    addr->sin_addr.s_addr = inet_addr("255.255.255.255");

    if (bind(bd_fd, (struct sockaddr *) addr, sizeof(struct sockaddr_in)) == ERROR) {
        log_warn("bind failed on port: %d!", send_port);
        return ERROR;
    }

    return LD_OK;
}

l_err init_udp_bd_recv(ld_dev_udp_para_t *udp_para, int recv_port) {
    int on = 1;

    int bd_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (bd_fd == ERROR) {
        log_error("Create broadcast socket failed !\n");
        return LD_ERR_INTERNAL;
    }

    if (setsockopt(bd_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == ERROR) {
        log_error("setsockopt failed!\n");
        return LD_ERR_INTERNAL;
    }

    if (config.role == LD_AS) {
        udp_para->fl_recv_fd = bd_fd;
    }else {
        udp_para->rl_recv_fd = bd_fd;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(recv_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(bd_fd, (struct sockaddr *) &addr, sizeof(addr)) == ERROR) {
        log_warn("bind failed on port: %d!", recv_port);
        return ERROR;
    }

    return LD_OK;
}

static l_err udp_send_msg(device_entity_t *arg, buffer_t *buf, ld_orient ori) {
    ld_dev_udp_para_t *udp_para = (ld_dev_udp_para_t *)arg;
    if (buf->len == 0) {
        return LD_ERR_NULL;
    }
    if (sendto(ori == FL ? udp_para->fl_send_fd : udp_para->rl_send_fd, buf->ptr, buf->len, 0,
               (struct sockaddr *) (ori == FL ? &udp_para->fl_send_addr : &udp_para->rl_send_addr),
               sizeof(struct sockaddr)) == ERROR) {
        return LD_ERR_INTERNAL;
    }
    return LD_OK;
}


static void *udp_recving_msgs(void *arg) {
    ld_dev_udp_para_t *udp_para = (ld_dev_udp_para_t *)arg;
    lfqueue_t *queue = udp_para->dev.msg_queue;
    int recv_fd;

    if (config.role == LD_AS) {
        recv_fd = udp_para->fl_recv_fd;
    } else {
        recv_fd = udp_para->rl_recv_fd;
    }

    while (stop_flag == FALSE) {
        uint8_t str[MAX_INPUT_BUFFER_SIZE] = {0};
        size_t len = recvfrom(recv_fd, str, MAX_INPUT_BUFFER_SIZE, 0, NULL, NULL);
        buffer_t *buf = init_buffer_unptr();
        if (len) {
            CLONE_TO_CHUNK(*buf, str, len)
            lfqueue_put(queue, buf);
        }else {
            free_buffer(buf);
        }
        zero(str);
    }
    return NULL;
}

static void *udp_recv_msg(void *arg) {
    ld_dev_udp_para_t *udp_para = (ld_dev_udp_para_t *)arg;
    // int *recv_fd = (config.role == LD_AS) ? &udp_para.fl_fd : &udp_para.rl_fd;
    if (config.role == LD_AS) {
        if (pthread_create(&udp_para->fl_th, NULL, udp_recving_msgs, udp_para) != 0) {
            log_error("Attacker FL Receiving thread create failed");
        }
        pthread_join(udp_para->fl_th, NULL);
    } else if (config.role == LD_GS) {
        if (pthread_create(&udp_para->rl_th, NULL, udp_recving_msgs, udp_para) != 0) {
            log_error("Attacker RL Receiving thread create failed");
        }
        pthread_join(udp_para->rl_th, NULL);
    }

    return NULL;
}

/**
 * set new port by frequency
 * @param arg
 * @param channel_num
 * @param ori
 * @return
 */
static l_err set_freq_port(device_entity_t *arg, int channel_num, ld_orient ori) {
    ld_dev_udp_para_t *udp_para = (ld_dev_udp_para_t *)arg;
    int port = REF_PORT + channel_num;
    switch (config.role) {
        case LD_AS: {
            if (ori == FL) {
                if (udp_para->fl_recv_fd != -1) {
                    close(udp_para->fl_recv_fd);
                }
                init_udp_bd_recv(udp_para, port);
                log_error("FL recv port %d, fd: %d", port, udp_para->fl_recv_fd);

                if (udp_para->rl_th) {
                    pthread_kill(udp_para->fl_th, 0);
                }
                if (pthread_create(&udp_para->fl_th, NULL, udp_recving_msgs, udp_para) != 0) {
                    log_error("Attacker FL Receiving thread create failed");
                }
                pthread_detach(udp_para->fl_th);
            } else {
                if (udp_para->rl_send_fd != -1) {
                    close(udp_para->rl_send_fd);
                }
                init_udp_bd_send(udp_para, port);
                log_error("RL send port %d, fd: %d", port, udp_para->rl_send_fd);
            }
            break;
        }
        case LD_GS: {
            if (ori == FL) {
                if (udp_para->fl_send_fd != -1) {
                    close(udp_para->fl_send_fd);
                }
                init_udp_bd_send(udp_para, port);
                log_info("FL send port %d, fd: %d", port, udp_para->fl_send_fd);
            } else {
                if (udp_para->rl_recv_fd != -1) {
                    close(udp_para->rl_recv_fd);
                }
                init_udp_bd_recv(udp_para, port);
                log_info("RL recv port %d, fd: %d", port, udp_para->rl_recv_fd);

                if (udp_para->rl_th) {
                    pthread_kill(udp_para->rl_th, 0);
                }
                if (pthread_create(&udp_para->rl_th, NULL, udp_recving_msgs, udp_para) != 0) {
                    log_error("Attacker RL Receiving thread create failed");
                }
                pthread_detach(udp_para->rl_th);
            }
            break;
        }
        default:
            return LD_ERR_WRONG_PARA;
    }
    return LD_OK;
}

/**
 * init udp simulation
 * @return
 */
ld_dev_udp_para_t *setup_udp_device(l_err (*process_func)(void *)) {
    /** mutual binding */
    ld_dev_udp_para_t *udp_para = calloc(1, sizeof(ld_dev_udp_para_t));
    udp_para->fl_send_fd = -1;
    udp_para->fl_recv_fd = -1;
    udp_para->rl_send_fd = -1;
    udp_para->rl_recv_fd = -1;

    udp_para->dev.send_pkt = udp_send_msg;
    udp_para->dev.recv_pkt = udp_recv_msg;
    udp_para->dev.set_freq = set_freq_port;

    udp_para->dev.name = "UDP";
    udp_para->dev.msg_queue = lfqueue_init();
    udp_para->dev.process_func = process_func;
    memset(udp_para->dev.freq_table, 0, CHANNEL_MAX);


    return udp_para;
}
