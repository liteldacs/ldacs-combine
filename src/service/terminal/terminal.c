//
// Created by 邹嘉旭 on 2024/6/10.
//

#include "service/terminal.h"

#include <ld_net.h>
#include <crypto/secure_core.h>

#include "ipv6_parse.h"

terminal_obj_t terminal_obj = {

};
#pragma pack(1)
typedef struct ipv6_tcp_s {
    uint8_t version;
    uint8_t traffic_class;
    uint32_t flow_label;
    uint16_t payload_len;
    uint8_t next_header;
    uint8_t hop_limit;
    buffer_t *src_address;
    buffer_t *dst_address;
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t sqn;
    uint32_t ack;
    uint8_t bias;
    uint8_t reserve;
    uint16_t flag;
    uint16_t window;
    uint16_t checksum;
    uint16_t urgent;
    buffer_t *data;
} ipv6_tcp_t;
#pragma pack()

static field_desc ipv6_tcp_fields[] = {
    {ft_set, 4, "VERSION", NULL},
    {ft_set, 8, "TRAFFIC CLASS", NULL},
    {ft_set, 20, "FLOW LABEL", NULL},
    {ft_set, 16, "PAYLOAD LEN", NULL},
    {ft_set, 8, "NEXT HEADER", NULL},
    {ft_set, 8, "HOP LIMIT", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_fl_str, 0, "IP SRC", &(pk_fix_length_t){.len = 16}},
    {ft_fl_str, 0, "IP DST", &(pk_fix_length_t){.len = 16}},
    {ft_set, 16, "SRC PORT", NULL},
    {ft_set, 16, "DST PORT", NULL},
    {ft_set, 32, "SQN", NULL},
    {ft_set, 32, "ACK", NULL},
    {ft_set, 4, "BIAS", NULL},
    {ft_set, 3, "PRESERVE", NULL},
    {ft_set, 9, "FLAG", NULL},
    {ft_set, 16, "WINDOW", NULL},
    {ft_set, 16, "CHECKSUM", NULL},
    {ft_set, 16, "URGENT", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_dl_str, 0, "DATA", NULL},
    {ft_pad, 0, "PAD", NULL},
    {ft_end, 0, NULL, NULL},
};
struct_desc_t ipv6_tcp_desc = {"TCP V6", ipv6_tcp_fields};

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

void *send_user_data_func(void *args) {
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

    inet_pton(AF_INET6, "3::1", &dst_addr);
    CLONE_TO_CHUNK(*v6.dst_address, dst_addr.__in6_u.__u6_addr8, GEN_ADDRLEN);

    CLONE_TO_CHUNK(*v6.data, "hello world", 11);
    buffer_t *buf = gen_pdu(&v6, &ipv6_tcp_desc, "TCP V6");

    log_buf(LOG_INFO, "IPV6", buf->ptr, buf->len);
    while (1) {
        sleep(5);
        send_user_data(buf->ptr, buf->len, terminal_obj.AS_SAC);
    }

    return NULL;
}


static void handle_st_chg_terminal(lme_state_chg_t *st_chg) {
    log_warn("The Current LME state is %d, by %.02x", st_chg->state, st_chg->sac);

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
    char msg[100] = {0};
    memcpy(msg, umsg->msg->ptr, umsg->msg->len);

    log_fatal("USER MESSAGE %d", umsg->msg->len);
}

static void send_singal_data_terminal(int argc, char **argv) {
    // char *test_msg = "Testing User Message for LDACS";
    // send_user_data((uint8_t *) test_msg, strlen(test_msg), terminal_obj.AS_SAC);
    char *data = "ABBA";
    char pkt[2048] = {0};
    // int pkt_len = construct_ipv6_udp_packet_to_char(config.addr,
    //                                                 "2001:da8:a012:389:7bf3:43b7:9c07:4f01", "5911", "5911", data, 4,
    //                                                 pkt);

    int pkt_len = construct_ipv6_udp_packet_to_char("2001:0:0:e304::141", config.addr,
                                                    "5911", "5911", data, 4,
                                                    pkt);

    send_user_data((uint8_t *) pkt, pkt_len, terminal_obj.AS_SAC);
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
    const char *test_msg = "Testing User Message for LDACS\0";
    for (int i = 0; i < 50; i++) {
        log_warn("Sending %d Packet===============", i+1);
        send_user_data((uint8_t *) test_msg, strlen(test_msg), terminal_obj.AS_SAC);
        usleep(250000);
    }
    // char *data = "ABBA";
    // char pkt[2048] = {0};
    // int pkt_len = construct_ipv6_udp_packet_to_char("2001:0:0:e304::141", config.addr,
    //                                                 "5911", "5911", data, 4,
    //                                                 pkt);
    //
    // send_user_data((uint8_t *) pkt, pkt_len, terminal_obj.AS_SAC);
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
