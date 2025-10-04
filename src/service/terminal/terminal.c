//
// Created by 邹嘉旭 on 2024/6/10.
//

#include "service/terminal.h"

#include <ld_net.h>
#include <crypto/secure_core.h>

#include "ipv6_parse.h"
#include <gmssl/rand.h>

terminal_obj_t terminal_obj = {

};


static l_err init_terminal_service();

static void handle_st_chg_terminal(lme_state_chg_t *);

static void handle_as_info_key_upd_terminal(as_info_key_upd_t *as_upd);

static void handle_as_info_upd_terminal(as_info_upd_t *as_info);

static void handle_user_msg_terminal(user_msg_t *umsg);

static void handle_update_coordinates_termianl(uint32_t AS_UA, double longitude, double latitude);

static void trigger_handover(int argc, char **argv);

static void send_singal_data_terminal(int argc, char **argv);

static void send_multi_data_terminal(int argc, char **argv);

static void send_specific_data_terminal(int argc, char **argv);

static void start_auth(int argc, char **argv);

static const size_t funcs_sz = 5;
static terminal_func terminal_funcs[] = {
    send_singal_data_terminal,
    trigger_handover,
    send_multi_data_terminal,
    send_specific_data_terminal,
    start_auth,
};

ld_service_t terminal_service = {
    .init_service = init_terminal_service,
    .handle_state_chg = handle_st_chg_terminal,
    .handle_as_info_key_upd = handle_as_info_key_upd_terminal,
    .handle_as_info_upd = handle_as_info_upd_terminal,
    .handle_recv_user_msg = handle_user_msg_terminal,
    .handle_update_coordinates = handle_update_coordinates_termianl,
};

static void monitor_terminal_input() {
    char input[1024];
    while (stop_flag == FALSE) {
        printf("> "); // 提示符
        fflush(stdout); // 确保提示符立即显示

        if (fgets(input, sizeof(input), stdin) == NULL || !strncmp(input, "exit", 4)) {
            kill(-getpid(), SIGINT);
            break;
        }

        // 去除末尾的换行符
        input[strcspn(input, "\n")] = '\0';

        if (strlen(input)) {
            char **argv;
            int argc = ld_split(input, &argv);
            if (argc == 0 || argv == NULL) { continue; }
            long serial = strtol(argv[0], NULL, 10);
            if (serial >= funcs_sz) {
                log_warn("Invalid func serial number");
                continue;
            }
            terminal_funcs[serial](argc, argv);
            free(argv);
        }

        // 处理输入过长导致缓冲区未读完的情况
        if (strlen(input) == sizeof(input) - 1 && input[sizeof(input) - 2] != '\n') {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF); // 清空缓冲区残留
        }
    }
}

static l_err init_terminal_service() {
    log_info("The ldacs simulator is using 'UNUSE HTTP' mode.");

    if (config.direct) {
        terminal_obj.AS_SAC = config.AS_SAC;
    }

    rcu_power_on(config.role); //power on directly
    start_auth(0, NULL);

    monitor_terminal_input();
    return LD_OK;
}

void *send_user_data_func(void *args) {
    buffer_t *buf = gen_ipv6_pkt(20);
    log_buf(LOG_INFO, "IPV6", buf->ptr, buf->len);
    while (1) {
        sleep(5);
        send_user_data_as(buf->ptr, buf->len);
    }
    return NULL;
}

static void handle_st_chg_terminal(lme_state_chg_t *st_chg) {
    // log_warn("The Current LME state is %d, by %.03x", st_chg->state, st_chg->sac);

    if (config.direct) {
        if (st_chg->state != LME_OPEN) return;

        pthread_create(&terminal_obj.data_th, NULL, send_user_data_func, NULL);
        pthread_detach(terminal_obj.data_th);
    }
}

static void handle_as_info_key_upd_terminal(as_info_key_upd_t *as_upd) {
    const char *tag = "AS_SAC";
    if (as_upd->key->len >= strlen(tag) && !memcmp(as_upd->key->ptr, tag, strlen(tag))) {
        terminal_obj.AS_SAC = as_upd->value;
    }
}

static void handle_as_info_upd_terminal(as_info_upd_t *as_info) {
}

static void handle_user_msg_terminal(user_msg_t *umsg) {
    char msg[2048] = {0};
    memcpy(msg, umsg->msg->ptr, umsg->msg->len);

    if (config.direct) {
        log_info("TCP Payload length %d", umsg->msg->len);
    }else {
        log_buf(LOG_INFO, "TCP Payload:", umsg->msg->ptr + 60, umsg->msg->len-60);
    }
}

static void handle_update_coordinates_termianl(uint32_t AS_UA, double longitude, double latitude) {
}

static void trigger_handover(int argc, char **argv) {
    if (argc != 3) {
        log_warn("Wrong paras");
        return;
    }
    uint32_t UA = strtol(argv[1], NULL, 10);
    uint32_t GST_SAC = strtol(argv[2], NULL, 10);
    rcu_handover(UA, GST_SAC);
}

static void send_singal_data_terminal(int argc, char **argv) {
    send_singal_data();
}

static void send_multi_data_terminal(int argc, char **argv) {
    send_multi_datas();
}

static void send_specific_data_terminal(int argc, char **argv) {
    if (argc != 2) {
        log_warn("Wrong paras");
        return;
    }
    uint32_t size = strtol(argv[1], NULL, 10);
    if (size > 2000) {
        log_warn("Too long");
        return;
    }
    uint8_t rand[2000] = {0};
    generate_nrand(rand, size);

    send_user_data_as(rand, size);
}


static void start_auth(int argc, char **argv) {
    if (config.role == LD_AS) rcu_start_auth();
    // else log_warn("Only AS can start auth");
}
