//
// Created by jiaxv on 25-10-3.
//
#include "service/service.h"
#include <ld_config.h>
#include <ipv6_parse.h>
#include <ld_net.h>
#include <crypto/secure_core.h>

#include "layer_interface.h"

// 生成包含A-Z a-z 0-9的随机字符串
void generate_random_string(char *str, size_t length) {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    int charset_size = sizeof(charset) - 1; // 减1是因为末尾的'\0'

    // 生成随机字符串
    for (int i = 0; i < length; i++) {
        int key = generate_urand(SYSTEM_BITS) % charset_size;
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

    inet_pton(AF_INET6, "2001::35e8:0017", &dst_addr);
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


void *send_user_data_func(void *args) {
    buffer_t *buf = gen_ipv6_pkt(generate_urand(SYSTEM_BITS) % 50);
    log_buf(LOG_INFO, "IPV6", buf->ptr, buf->len);
    while (1) {
        sleep(5);
        send_user_data_as(buf->ptr, buf->len);
    }
    return NULL;
}

#pragma pack(1)
typedef struct ld_keystore_s{
    buffer_t *id;
    buffer_t *type;
    buffer_t *owner1;
    buffer_t *owner2;
    buffer_t *key_cipher;
    uint32_t key_len;
    buffer_t *key_state;
    buffer_t *create_time;
    uint32_t update_cycle;
    uint32_t kek_len;
    buffer_t * kek_cipher;
    buffer_t * iv;
    uint32_t iv_len;
    buffer_t *check_algo;
    uint32_t check_len;
    buffer_t *check_value;
    uint32_t update_count;
}ld_keystore_t;
#pragma pack()

static l_err init_keystore(ld_keystore_t *key) {
    if (key == NULL) {
        return LD_ERR_NULL;
    }

    key->id = init_buffer_unptr();
    key->type = init_buffer_unptr();
    key->owner1 = init_buffer_unptr();
    key->owner2 = init_buffer_unptr();
    key->key_cipher = init_buffer_unptr();
    key->key_state = init_buffer_unptr();
    key->create_time = init_buffer_unptr();
    key->kek_cipher = init_buffer_unptr();
    key->iv = init_buffer_unptr();
    key->check_algo = init_buffer_unptr();
    key->check_value = init_buffer_unptr();

    key->key_len = 0;
    key->update_cycle = 0;
    key->kek_len = 0;
    key->iv_len = 0;
    key->check_len = 0;
    key->update_count = 0;

    return LD_OK; // 成功
}

static void free_keystore(ld_keystore_t *key) {
    if (!key)   return;

    if (key->id)    free_buffer(key->id);
    if (key->type)  free_buffer(key->type);
    if (key->owner1) free_buffer(key->owner1);
    if (key->owner2) free_buffer(key->owner2);
    if (key->key_cipher) free_buffer(key->key_cipher);
    if (key->key_state) free_buffer(key->key_state);
    if (key->create_time) free_buffer(key->create_time);
    if (key->kek_cipher) free_buffer(key->kek_cipher);
    if (key->iv) free_buffer(key->iv);
    if (key->check_algo) free_buffer(key->check_algo);
    if (key->check_value) free_buffer(key->check_value);

}

static ld_keystore_t *init_keystores(size_t sz) {
    ld_keystore_t *keys = malloc(sizeof(ld_keystore_t) * sz);

    for (int i = 0; i < sz; i++) {
        init_keystore(keys+i);
    }

    return keys;
}

static void free_keystores(ld_keystore_t *keys, size_t sz) {
    for (int i = 0; i < sz; i++) {
        free_keystore(keys+i);
    }
    free(keys);
}


void query_keys(int argc, char **argv) {
    sqlite3 *db;
    char *err_msg = 0;
    int rc;

    // 打开数据库连接
    rc = sqlite3_open("/home/jiaxv/.ldcauc/as_keys_001162345.db", &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return ;
    }

    // 执行 SELECT * 查询
    const char *sql = "SELECT * FROM as_keystore;";
    sqlite3_stmt *stmt;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        ld_keystore_t *keys = init_keystores(200);
        size_t key_count = 0;

        // 遍历结果集
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            CLONE_BY_CHARS(*keys[key_count].id, (const uint8_t*)sqlite3_column_text(stmt, 0))
            CLONE_BY_CHARS(*keys[key_count].type, (const uint8_t*)sqlite3_column_text(stmt, 1))
            CLONE_BY_CHARS(*keys[key_count].owner1, (const uint8_t*)sqlite3_column_text(stmt, 2))
            CLONE_BY_CHARS(*keys[key_count].owner2, (const uint8_t*)sqlite3_column_text(stmt, 3))
            CLONE_BY_CHARS(*keys[key_count].key_cipher, (const uint8_t*)sqlite3_column_text(stmt, 4))
            keys[key_count].key_len = sqlite3_column_int(stmt, 5);
            CLONE_BY_CHARS(*keys[key_count].key_state, (const uint8_t*)sqlite3_column_text(stmt, 6))
            CLONE_BY_CHARS(*keys[key_count].create_time, (const uint8_t*)sqlite3_column_text(stmt, 7))
            keys[key_count].update_cycle = sqlite3_column_int(stmt, 8);
            keys[key_count].kek_len = sqlite3_column_int(stmt, 9);
            CLONE_BY_CHARS(*keys[key_count].kek_cipher, (const uint8_t*)sqlite3_column_text(stmt, 10))
            CLONE_BY_CHARS(*keys[key_count].iv, (const uint8_t*)sqlite3_column_text(stmt, 11))
            keys[key_count].iv_len = sqlite3_column_int(stmt, 12);
            CLONE_BY_CHARS(*keys[key_count].check_algo, (const uint8_t*)sqlite3_column_text(stmt, 13))
            keys[key_count].check_len = sqlite3_column_int(stmt, 14);
            CLONE_BY_CHARS(*keys[key_count].check_value, (const uint8_t*)sqlite3_column_text(stmt, 15))
            keys[key_count].update_count = sqlite3_column_int(stmt, 16);

            log_buf(LOG_ERROR, "ID:", keys[key_count].id->ptr, keys[key_count].id->len);
            key_count++;
        }

        free_keystores(keys, 200);
    } else {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }

    // 清理资源
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}
