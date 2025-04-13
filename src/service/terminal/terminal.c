//
// Created by 邹嘉旭 on 2024/6/10.
//

#include "service/terminal.h"

terminal_obj_t terminal_obj = {

};

static l_err init_terminal_service();

static void handle_st_chg_terminal(lme_state_chg_t *);

static void handle_as_info_key_upd_terminal(as_info_key_upd_t *as_upd);

static void handle_as_info_upd_terminal(as_info_upd_t *as_info);

static void handle_user_msg_terminal(user_msg_t *umsg);

static void send_user_data_terminal();

static const size_t funcs_sz = 1;
static terminal_func terminal_funcs[] = {
        send_user_data_terminal
};

ld_service_t terminal_service = {
        .init_service = init_terminal_service,
        .handle_state_chg = handle_st_chg_terminal,
        .handle_as_info_key_upd = handle_as_info_key_upd_terminal,
        .handle_as_info_upd = handle_as_info_upd_terminal,
        .handle_recv_user_msg = handle_user_msg_terminal,
};

static void listen_terminal_input() {
    char input[1024];
    while (stop_flag == FALSE) {
        printf("> ");          // 提示符
        fflush(stdout);        // 确保提示符立即显示

        if (fgets(input, sizeof(input), stdin) == NULL || !strncmp(input, "exit", 4)) {
            kill(-getpid(), SIGINT);
            break;
        }

        // 去除末尾的换行符
        input[strcspn(input, "\n")] = '\0';

        if (strlen(input)) {
//            printf("接收到输入: %s\n", input);
            long serial = strtol(input, NULL, 10);
            if (serial >= funcs_sz) {
                log_warn("Invalid func serial number");
                continue;
            }
            terminal_funcs[serial](NULL);
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

    rcu_power_on(config.role); //power on directly
    if (config.role == LD_AS) rcu_start_auth();

    listen_terminal_input();
    return LD_OK;
}


static void handle_st_chg_terminal(lme_state_chg_t *st_chg) {
    // log_warn("The Current LME state is %d, by %.02x", st_chg->state, st_chg->sac);
}

static void handle_as_info_key_upd_terminal(as_info_key_upd_t *as_upd) {
    log_warn("NEW value of AS key `%s` is `%x`", as_upd->key->ptr, as_upd->value);
    const char *tag = "AS_SAC";
    if (as_upd->key->len >= strlen(tag) && !memcmp(as_upd->key->ptr, tag, strlen(tag))) {
        terminal_obj.AS_SAC = as_upd->value;
//        log_warn("!!!!!!!!!!!!!! %d", terminal_obj.AS_SAC);
    }
}

static void handle_as_info_upd_terminal(as_info_upd_t *as_info) {
//    log_fatal("NEW value of AS by %.02x", as_info->AS_SAC);
}

static void handle_user_msg_terminal(user_msg_t *umsg) {
    log_fatal("USER MESSAGE %s", umsg->msg->ptr);
}

static void send_user_data_terminal() {
    char *test_msg = "BBBBBBBBBBBBBBBBBBB\0";
    send_user_data((uint8_t *) test_msg, strlen(test_msg), terminal_obj.AS_SAC);
}
