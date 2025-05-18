//
// Created by 邹嘉旭 on 2023/12/20.
//

#include "device/device_udp.h"

static ld_dev_udp_para_t udp_para;

static int init_udp_bd_send(struct sockaddr_in *addr_p, int send_port) {
    int on = 1;
    int bd_fd;

    bd_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (bd_fd == ERROR) {
        log_error("Create broadcast socket failed !\n");
        return ERROR;
    }

    if (setsockopt(bd_fd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) == ERROR) {
        log_error("setsockopt failed!\n");
        return ERROR;
    }

    bzero(addr_p, sizeof(struct sockaddr_in));
    addr_p->sin_family = AF_INET;
    addr_p->sin_port = htons(send_port);
    addr_p->sin_addr.s_addr = inet_addr("255.255.255.255");

    return bd_fd;
}

static int init_udp_bd_recv(int recv_port) {
    int on = 1;
    int bd_fd;

    bd_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (bd_fd == ERROR) {
        log_error("Create broadcast socket failed !\n");
        return ERROR;
    }

    if (setsockopt(bd_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == ERROR) {
        log_error("setsockopt failed!\n");
        return ERROR;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(recv_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(bd_fd, (struct sockaddr *) &addr, sizeof(addr)) == ERROR) {
        log_warn("bind failed !\n");
        return ERROR;
    }

    return bd_fd;
}

static l_err udp_send_msg(buffer_t *buf, ld_orient ori) {
    if (buf->len == 0) {
        return LD_ERR_NULL;
    }
    if (sendto(ori == FL ? udp_para.fl_send_fd : udp_para.rl_send_fd, buf->ptr, buf->len, 0,
               (struct sockaddr *) (ori == FL ? &udp_para.fl_send_addr : &udp_para.rl_send_addr),
               sizeof(struct sockaddr)) == ERROR) {
        return LD_ERR_INTERNAL;
    }
    return LD_OK;
}


static void *udp_recving_msgs(void *arg) {
    lfqueue_t *queue = udp_para.dev->msg_queue;
    int recv_fd = *(int *) arg;
    while (stop_flag == FALSE) {
        uint8_t str[MAX_INPUT_BUFFER_SIZE] = {0};
        size_t len = recvfrom(recv_fd, str, MAX_INPUT_BUFFER_SIZE, 0, NULL, NULL);
        buffer_t *buf = init_buffer_unptr();
        if (len) {
            CLONE_TO_CHUNK(*buf, str, len)
            lfqueue_put(queue, buf);
        }
        zero(str);
    }
    return NULL;
}

static void *udp_recv_msg(void *arg) {
    pthread_t fl_th, rl_th;
    // int *recv_fd = (config.role == LD_AS) ? &udp_para.fl_fd : &udp_para.rl_fd;
    if (config.role != LD_GS) {
        if (pthread_create(&fl_th, NULL, udp_recving_msgs, &udp_para.fl_recv_fd) != 0) {
            log_error("Attacker FL Receiving thread create failed");
        }
        pthread_join(fl_th, NULL);
    } else if (config.role != LD_AS) {
        if (pthread_create(&rl_th, NULL, udp_recving_msgs, &udp_para.rl_recv_fd) != 0) {
            log_error("Attacker RL Receiving thread create failed");
        }
        pthread_join(rl_th, NULL);
    }


    return NULL;
}

/**
 * set new port by frequency
 * @param channel_num
 * @param ori
 * @return
 */
static l_err set_freq_port(int channel_num, ld_orient ori) {
    int port = REF_PORT + channel_num;
    switch (config.role) {
        case LD_AS: {
            if (ori == FL) {
                if (udp_para.fl_recv_fd != -1) {
                    close(udp_para.fl_recv_fd);
                }
                udp_para.fl_recv_fd = init_udp_bd_recv(port);
            } else {
                if (udp_para.rl_send_fd != -1) {
                    close(udp_para.rl_send_fd);
                }
                udp_para.rl_send_fd = init_udp_bd_send(&udp_para.rl_send_addr, port);
            }
            break;
        }
        case LD_GS: {
            if (ori == FL) {
                if (udp_para.fl_send_fd != -1) {
                    close(udp_para.fl_send_fd);
                }
                udp_para.fl_send_fd = init_udp_bd_send(&udp_para.fl_send_addr, port);
            } else {
                if (udp_para.rl_recv_fd != -1) {
                    close(udp_para.rl_recv_fd);
                }
                udp_para.rl_recv_fd = init_udp_bd_recv(port);
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
 * @param en
 * @return
 */
l_err set_udp_device(ld_dev_entity_t *en) {
    /** mutual binding */
    udp_para.dev = en;
    udp_para.fl_send_fd = -1;
    udp_para.fl_recv_fd = -1;
    udp_para.rl_send_fd = -1;
    udp_para.rl_recv_fd = -1;

    en->dev_para = &udp_para;
    en->send_pkt = udp_send_msg;
    en->recv_pkt = udp_recv_msg;
    en->set_freq = set_freq_port;

    return LD_OK;
}
