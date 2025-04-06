//
// Created by 邹嘉旭 on 2024/4/25.
//

#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "http_core.h"
#define GET_QUERY_SIZE  128
#define POST_DATA_SIZE  1024
typedef char QUERY[GET_QUERY_SIZE];

#pragma pack(1)

typedef struct post_test_s {
    uint8_t page;
    buffer_t *msg;
    uint8_t test2;
} post_test_t;

typedef struct post_open_s {
    uint8_t role;
} post_open_t;

typedef struct post_null_s {
} post_null_t;

typedef struct post_gs_key_upd_s {
    uint16_t sac;
} post_gs_key_upd_t;

typedef struct post_as_send_msg_s {
    uint16_t sac;
    buffer_t *msg;
} post_as_send_msg_t;

#pragma pack()

/* struct for GET input is defined out of #pragma pack() */
typedef struct test_get_req_s {
    char *page;
    char *test_2;
    char *test_3;
    char *aaa;
} test_get_req_t;


typedef struct get_null_s {
} get_null_t;


extern json_tmpl_desc_t post_test_tmpl_desc;
extern json_tmpl_desc_t post_open_tmpl_desc;
extern json_tmpl_desc_t post_null_tmpl_desc;
extern json_tmpl_desc_t post_as_send_msg_tmpl_desc;
extern json_tmpl_desc_t post_gs_key_upd_tmpl_desc;

#define DEFUN_HTTP_API(name)                                              \
    static void name(struct evhttp_request *req, void *arg)

#define DEFUN_HTTP_GET_API(name, query_type, body)                                              \
    static void name(struct evhttp_request *req, void *arg) {\
        if (req == NULL) return;\
        query_type qstruct;\
        if (parse_get_query(req, &qstruct)) { \
            return;\
        }\
        body\
    }

#define DEFUN_HTTP_POST_API(name, data_type, desc, body)  \
    static void name(struct evhttp_request *req, void *arg) {\
        data_type dstruct;\
        zero(&dstruct);\
        if (parse_post_data(req, &dstruct, &desc)) {\
            log_error("WRONG!");\
            return;\
        }\
        do{ \
            body\
        }while (0);\
    }

l_err init_http_controllers(struct evhttp *hentity);

#endif //HTTP_REQUEST_H
