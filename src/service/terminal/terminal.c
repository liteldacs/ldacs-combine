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

static void send_singal_data_terminal(int argc, char **argv);

static void trigger_handover(int argc, char **argv);

static void send_multi_data_terminal(int argc, char **argv);

static void send_specific_data_terminal(int argc, char **argv);

static void send_bdsbas_data_terminal(int argc, char **argv);

static const size_t funcs_sz = 5;
static terminal_func terminal_funcs[] = {
    send_singal_data_terminal,
    trigger_handover,
    send_multi_data_terminal,
    send_specific_data_terminal,
    send_bdsbas_data_terminal,
};

ld_service_t terminal_service = {
    .init_service = init_terminal_service,
    .handle_state_chg = handle_st_chg_terminal,
    .handle_as_info_key_upd = handle_as_info_key_upd_terminal,
    .handle_as_info_upd = handle_as_info_upd_terminal,
    .handle_recv_user_msg = handle_user_msg_terminal,
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
    if (config.role == LD_AS) rcu_start_auth();

    monitor_terminal_input();
    return LD_OK;
}

// 生成包含A-Z a-z 0-9的随机字符串
static void generate_random_string(char *str, size_t length) {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    int charset_size = sizeof(charset) - 1; // 减1是因为末尾的'\0'

    // 初始化随机数种子
    srand((unsigned int)time(NULL));

    // 生成随机字符串
    for (int i = 0; i < length; i++) {
        int key = rand() % charset_size;
        str[i] = charset[key];
    }

    // 字符串结束符
    str[length] = '\0';
}

buffer_t *gen_ipv6_pkt(size_t len) {
    ipv6_tcp_t v6 = {
        .version = 0x6,
        .traffic_class = 0x3,
        .flow_label = 0x1,
        .payload_len = 11,
        .next_header = 0x7,
        .hop_limit = 0x10,
        .src_address = init_buffer_unptr(),
        .dst_address = init_buffer_unptr(),
        .src_port = 9456,
        .dst_port = 9789,
        .sqn = 1000,
        .ack = 2000,
        .bias = 0,
        .reserve = 0x1,
        .flag = 0x2,
        .window = 500,
        .checksum = 0xABCD,
        .urgent = 0,
        .data = init_buffer_unptr()
    };

    struct in6_addr src_addr, dst_addr;
    inet_pton(AF_INET6, config.addr, &src_addr);
    CLONE_TO_CHUNK(*v6.src_address, src_addr.__in6_u.__u6_addr8, GEN_ADDRLEN);

    inet_pton(AF_INET6, "2001::E304", &dst_addr);
    CLONE_TO_CHUNK(*v6.dst_address, dst_addr.__in6_u.__u6_addr8, GEN_ADDRLEN);

    size_t curr = 0;
    uint8_t msg[1500];
    // for (int j = 0; j < len / RAND_BYTES_MAX_SIZE; j++) {
    //     km_generate_random(msg + curr, RAND_BYTES_MAX_SIZE);
    //     curr += RAND_BYTES_MAX_SIZE;
    // }
    // km_generate_random(msg + curr, len % RAND_BYTES_MAX_SIZE);

    generate_random_string(msg, len);

    CLONE_TO_CHUNK(*v6.data, msg, len);

    return gen_pdu(&v6, &ipv6_tcp_desc, "TCP V6");
}

void *send_user_data_func(void *args) {
    buffer_t *buf = gen_ipv6_pkt(20);
    log_buf(LOG_INFO, "IPV6", buf->ptr, buf->len);
    while (1) {
        sleep(5);
        send_user_data(buf->ptr, buf->len, terminal_obj.AS_SAC);
    }

    return NULL;
}


static void handle_st_chg_terminal(lme_state_chg_t *st_chg) {
    log_warn("The Current LME state is %d, by %.03x", st_chg->state, st_chg->sac);

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
        log_info("用户数据长度 %d", umsg->msg->len);
    }else {
        char payload[2048] = {0};
        memcpy(payload, umsg->msg->ptr + 60, umsg->msg->len-60);
        log_info("获取 TCP Payload: %s", payload);
    }
}

static void send_singal_data_terminal(int argc, char **argv) {
    buffer_t *buf = gen_ipv6_pkt(20);

    send_user_data(buf->ptr, buf->len, terminal_obj.AS_SAC);
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

static void send_multi_data_terminal(int argc, char **argv) {

    if (config.direct) {
        log_warn("Direct mode cant send multiple message");
        return;
    }

    // const char *test_msg = "Testing User Message for LDACS\0";
    for (int i = 1; i <= 10; i++) {
        buffer_t *buf = gen_ipv6_pkt(i*100);
        log_warn("Sending %d Packet===============", i);
        send_user_data(buf->ptr, buf->len, terminal_obj.AS_SAC);
        usleep(250000);
    }
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

    send_user_data(rand, size, terminal_obj.AS_SAC);
}

static void send_bdsbas_data_terminal(int argc, char **argv) {
    uint8_t bdsbas_data[32] = {
        0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x74, 0x3E, 0x00
    };
    send_user_data(bdsbas_data, sizeof(bdsbas_data), terminal_obj.AS_SAC);
}
