//
// Created by 邹嘉旭 on 2024/5/6.
//

#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H
#include "http_core.h"

enum HTTP_DATA_CODE {
    HTTP_DATA_OK = 0,
    HTTP_DATA_ERROR = 7
};


#define OUT_HEAD \
    { \
    struct evkeyvalq *outhead = evhttp_request_get_output_headers(req); \
    evhttp_add_header(outhead, "Content-Type", "application/json"); \
    evhttp_add_header(outhead, "Access-Control-Allow-Origin", "*"); \
    }

#pragma pack(1)
typedef struct http_resp_data_s {
    uint8_t code;
    char *data;
    buffer_t *msg;
} http_resp_data_t;

typedef struct lme_state_resp_s {
    uint32_t state;
} lme_state_resp_t;

typedef struct rcu_state_resp_s {
    uint32_t state;
} rcu_state_resp_t;

typedef struct occupy_rcu_resp_s {
    uint8_t state;
} occupy_rcu_resp_t;


#pragma pack()


extern json_tmpl_desc_t http_resp_data_tmpl_desc;
extern json_tmpl_desc_t lme_state_resp_tmpl_desc;
extern json_tmpl_desc_t rcu_state_resp_tmpl_desc;
extern json_tmpl_desc_t occupy_rcu_resp_tmpl_desc;

void http_resp_send(struct evhttp_request *req, enum HTTP_DATA_CODE code, void *data_p, json_tmpl_desc_t *desc,
                    const char *msg, const char *sse_type);

#define HTTP_SEND_OK \
    http_resp_send(req, HTTP_DATA_OK, NULL, NULL, "OK", NULL);

#define HTTP_SEND_OK_WITH_MSG(msg) \
    http_resp_send(req, HTTP_DATA_OK, NULL, NULL, msg, NULL);

#define HTTP_SEND_OK_WITH_DATA(data, desc) \
    http_resp_send(req, HTTP_DATA_OK, data, desc, NULL, NULL);

#define HTTP_SEND_OK_WITH_DETAIL(data, desc, msg) \
    http_resp_send(req, HTTP_DATA_OK, data, desc, msg, NULL);

#define HTTP_SEND_FAILED \
    http_resp_send(req, HTTP_DATA_ERROR, NULL, NULL, "FAILED", NULL);

#define HTTP_SEND_FAILED_WITH_MSG(msg) \
    http_resp_send(req, HTTP_DATA_ERROR, NULL, NULL, msg, NULL);

#define HTTP_SEND_FAILED_WITH_DETAIL(data, desc, msg) \
    http_resp_send(req, HTTP_DATA_ERROR, data, desc, msg, NULL);

#define HTTP_SSE_SEND_OK_WITH_DETAIL(data, desc, msg, type) \
    http_resp_send(req, HTTP_DATA_OK, data, desc, msg, type);

#define HTTP_SSE_SEND_FAILED_WITH_DETAIL(data, desc, msg, type) \
    http_resp_send(req, HTTP_DATA_ERROR, data, desc, msg, type);


void handle_st_chg_http(lme_state_chg_t *);

void handle_as_info_key_upd_http(as_info_key_upd_t *as_upd);

void handle_as_info_upd_http(as_info_upd_t *as_info);

void handle_user_msg_http(user_msg_t *umsg);

#endif //HTTP_RESPONSE_H
