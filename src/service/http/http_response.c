//
// Created by 邹嘉旭 on 2024/5/22.
//
#include "http_response.h"

static json_tmpl_t http_resp_data_tmpl[] = {
    {cJSON_Number, sizeof(uint8_t), "code", "code", NULL},
    {cJSON_Raw, sizeof(char *), "data", "data", NULL},
    {cJSON_String, sizeof(buffer_t *), "msg", "msg", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

static json_tmpl_t lme_state_resp_tmpl[] = {
    {cJSON_Number, sizeof(uint32_t), "state_id", "state_id", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

static json_tmpl_t rcu_state_resp_tmpl[] = {
    {cJSON_Number, sizeof(uint32_t), "state_id", "state_id", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

static json_tmpl_t occupy_rcu_resp_tmpl[] = {
    {cJSON_Number, sizeof(uint8_t), "state", "state_id", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};


json_tmpl_desc_t http_resp_data_tmpl_desc = {
    .desc = "HTTP_RESP_DATA",
    .tmpl = http_resp_data_tmpl,
    .size = sizeof(http_resp_data_t)
};

json_tmpl_desc_t lme_state_resp_tmpl_desc = {
    .desc = "LME_STATE_RESP",
    .tmpl = lme_state_resp_tmpl,
    .size = sizeof(lme_state_resp_t)
};

json_tmpl_desc_t rcu_state_resp_tmpl_desc = {
    .desc = "RCU_STATE_RESP",
    .tmpl = rcu_state_resp_tmpl,
    .size = sizeof(rcu_state_resp_t)
};

json_tmpl_desc_t occupy_rcu_resp_tmpl_desc = {
    .desc = "OCCUPY_RCU",
    .tmpl = occupy_rcu_resp_tmpl,
    .size = sizeof(occupy_rcu_resp_t)
};

/**
 * Sending Http response
 * @param req libevent http request object
 * @param code response code: 0 for OK and 7 for Error
 * @param data_p (char*) or struct that will be assembled into json
 * @param desc assenble-discription, only used when data_p is a struct
 * @param msg response message
 * @param sse_type
 */
void http_resp_send(struct evhttp_request *req, enum HTTP_DATA_CODE code, void *data_p, json_tmpl_desc_t *desc,
                    const char *msg, const char *sse_type) {
    buffer_t *buf = init_buffer_unptr();
    struct evbuffer *outbuf = evbuffer_new();
    char *data_str = NULL;

    if (desc == NULL) {
        /* change data into char* directly */
        data_str = data_p;
    } else {
        get_json_str(data_p, desc, &data_str);
    }
    if (msg != NULL) CLONE_TO_CHUNK(*buf, msg, strlen(msg));

    http_resp_data_t to_resp = {
        .code = code,
        .data = data_str,
        .msg = buf,
    };

    char *root_s;
    get_json_str(&to_resp, &http_resp_data_tmpl_desc, &root_s);

    if (sse_type == NULL) {
        OUT_HEAD;
        evbuffer_add(outbuf, root_s, strlen(root_s));
        evhttp_send_reply(req, HTTP_OK, "", outbuf);
    } else {
        evbuffer_add_printf(outbuf, "event: %s\ndata:%s\n\n", sse_type, root_s);
        evhttp_send_reply_chunk(req, outbuf);
    }

    if (desc != NULL) {
        free(data_str);
    }
    evbuffer_free(outbuf);
    free_buffer(buf);
    free(root_s);
}


void handle_st_chg_http(lme_state_chg_t *st_chg) {
    struct evhttp_request *req = http_server.sse_req;
    if (!req) return;
    HTTP_SSE_SEND_OK_WITH_DETAIL(st_chg, &sse_state_tmpl_desc, "", "state_change");
}

/* 更新单个as man 键值对 */
void handle_as_info_key_upd_http(as_info_key_upd_t *as_upd) {
    struct evhttp_request *req = http_server.sse_req;
    if (!req) return;
    HTTP_SSE_SEND_OK_WITH_DETAIL(as_upd, &as_info_key_upd_tmpl_desc, "", "as_key_update");
}

void handle_as_info_upd_http(as_info_upd_t *as_info) {
    struct evhttp_request *req = http_server.sse_req;
    if (!req) return;
    HTTP_SSE_SEND_OK_WITH_DETAIL(as_info, &as_info_upd_tmpl_desc, "", "as_update");
}

void handle_user_msg_http(user_msg_t *umsg) {
    struct evhttp_request *req = http_server.sse_req;
    if (!req) return;
    HTTP_SSE_SEND_OK_WITH_DETAIL(umsg, &user_msg_tmpl_desc, "", "recv_user_msg");
}

void handle_update_coordinates_http(uint32_t AS_UA, double longitude, double latitude) {

}
