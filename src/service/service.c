//
// Created by jiaxv on 25-10-3.
//
#include "service/service.h"
#include <ld_config.h>
#include <ipv6_parse.h>
#include <ld_net.h>

#include "layer_interface.h"

// 生成包含A-Z a-z 0-9的随机字符串
void generate_random_string(char *str, size_t length) {
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

    uint8_t msg[1800];
    generate_random_string(msg, len);
    CLONE_TO_CHUNK(*v6.data, msg, len);

    return gen_pdu(&v6, &ipv6_tcp_desc, "TCP V6");
}

void send_singal_data() {
    buffer_t *buf = gen_ipv6_pkt(20);

    send_user_data_as(buf->ptr, buf->len);

    if (!config.direct) {
        log_buf(LOG_INFO, "TCP Payload:", buf->ptr + 60, buf->len-60);
    }
}

#define PKT_COUNT 12
void send_multi_datas() {
    if (config.direct) {
        log_warn("Direct mode cant send multiple message");
        return;
    }

    int pkt_lens[PKT_COUNT] = {128, 256, 384, 512, 640, 768, 896, 1024, 1152, 1280, 1408, 1536};

    // const char *test_msg = "Testing User Message for LDACS\0";
    for (int i = 0; i < PKT_COUNT; i++) {
        buffer_t *buf = gen_ipv6_pkt(pkt_lens[i]);
        log_warn("Sending %d Packet===============", i + 1);
        send_user_data_as(buf->ptr, buf->len);
        usleep(250000);
    }
}

