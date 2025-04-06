//
// Created by 邹嘉旭 on 2024/4/25.
//

#include "http_request.h"

#include <layer_interface.h>
#include <layer_rcu.h>


static json_tmpl_t post_test_tmpl[] = {
    {cJSON_Number, sizeof(uint8_t), "page", "page", NULL},
    {cJSON_String, sizeof(buffer_t *), "msg", "msg", NULL},
    {cJSON_Number, sizeof(uint8_t), "test2", "test2", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

static json_tmpl_t post_open_tmpl[] = {
    {cJSON_Number, sizeof(uint8_t), "role", "role", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

static json_tmpl_t post_null_tmpl[] = {
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

static json_tmpl_t post_as_send_msg_tmpl[] = {
    {cJSON_Number, sizeof(uint16_t), "sac", "sac", NULL},
    {cJSON_String, sizeof(buffer_t *), "msg", "msg", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

static json_tmpl_t post_gs_key_upd_tmpl[] = {
    {cJSON_Number, sizeof(uint16_t), "sac", "sac", NULL},
    {cJSON_Invalid, 0, NULL, NULL, NULL}
};

json_tmpl_desc_t post_test_tmpl_desc = {
    .desc = "HTTP_POST_TEST",
    .tmpl = post_test_tmpl,
    .size = sizeof(post_test_t)
};

json_tmpl_desc_t post_open_tmpl_desc = {
    .desc = "HTTP_POST_OPEN",
    .tmpl = post_open_tmpl,
    .size = sizeof(post_open_t)
};

json_tmpl_desc_t post_null_tmpl_desc = {
    .desc = "HTTP_POST_NULL",
    .tmpl = post_null_tmpl,
    .size = sizeof(post_null_t)
};
json_tmpl_desc_t post_as_send_msg_tmpl_desc = {
    .desc = "HTTP_POST_AS_SENDMSG",
    .tmpl = post_as_send_msg_tmpl,
    .size = sizeof(post_as_send_msg_t)
};

json_tmpl_desc_t post_gs_key_upd_tmpl_desc = {
    .desc = "HTTP_POST_GS_KEYUPD",
    .tmpl = post_gs_key_upd_tmpl,
    .size = sizeof(post_gs_key_upd_t)
};

static l_err parse_get_query(struct evhttp_request *req, void *qstruct);

static l_err parse_post_data(struct evhttp_request *req, void *struct_ptr, json_tmpl_desc_t *desc);

static void http_source_req_cb(struct evhttp_request *request, void *arg);

/** http接口编写规范：
 *  GET接口:
 *      使用DEFUN_HTTP_GET_API宏
 *          para1: API名
 *          para2: API数据类型
 *          para3: 业务代码
 *      API数据变量名为 “qstruct”
 *  POST接口：
 *      使用DEFUN_HTTP_POST_API宏：
 *          para1: API名
 *          para2: API数据类型
 *          para3: 数据类型模板，用于解析请求携带数据
 *          para4: 业务代码
 *      API数据变量名为 “dstruct”
 *
 *  接口API命名规范：
 *      h_(role)_(http method)_(function):
 *          ‘h’:            HTTP接口须使用"h"为起始，表明该函数为HTTP接口
 *          role:           角色，基本角色类型为如下四种: 1."g" (global)适用于全局的 / 2."as" / 3."gs" / 4."gsc"
 *          http_method:    post或者get
 *          function:       具体功能
 *  返回规范：
 *      使用
 */

/**
 *
 * @param req
 * @param arg
 */

DEFUN_HTTP_POST_API(h_g_post_open_device, post_open_t, post_open_tmpl_desc, {
                    l_rcu_err err;
                    do {

                    if ((err = rcu_power_on(dstruct.role))) break;
                    if (config.role == LD_AS && config.auto_auth == TRUE) {
                    if ((err = rcu_start_auth())) break;
                    }

                    HTTP_SEND_OK_WITH_MSG("协议栈已开启");
                    } while (0);

                    HTTP_SEND_FAILED_WITH_MSG("协议栈开启失败");
                    }
)

DEFUN_HTTP_POST_API(h_g_post_occupy_rcu, post_null_t, post_null_tmpl_desc, {

                    l_rcu_err err = LD_RCU_OK;
                    do {
                    if ((err = rcu_is_occupied())) break;

                    //unoccupied -> occupied
                    rcu_change_occupied(TRUE);
                    HTTP_SEND_OK_WITH_DATA(&(occupy_rcu_resp_t){
                        .state = err
                        }, &occupy_rcu_resp_tmpl_desc);
                    }while (0);
                    HTTP_SEND_FAILED_WITH_DETAIL(&(occupy_rcu_resp_t){
                        .state = err
                        }, &occupy_rcu_resp_tmpl_desc, "RCU已被其他用户占用");
                    })

DEFUN_HTTP_GET_API(h_g_get_rcu_state, test_get_req_t, {
                   HTTP_SEND_OK_WITH_DATA(&(rcu_state_resp_t) {
                       .state = rcu_get_rcu_state()
                       }, &rcu_state_resp_tmpl_desc);
                   })

DEFUN_HTTP_POST_API(h_g_post_rcu_stop, post_null_t, post_null_tmpl_desc, {
                    if(rcu_is_occupied() == LD_RCU_OCCUPIED) {
                    rcu_change_occupied(FALSE);
                    }
                    HTTP_SEND_OK;
                    })

DEFUN_HTTP_GET_API(h_as_get_test, test_get_req_t, {
                   HTTP_SEND_OK_WITH_DETAIL("{\"item\": \"java\"}", NULL, "HELLO");
                   })

DEFUN_HTTP_POST_API(h_as_post_test, post_test_t, post_test_tmpl_desc, {
                    HTTP_SEND_OK_WITH_DETAIL("{\"item\": \"java\"}", NULL, "HELLO");
                    })

DEFUN_HTTP_POST_API(h_as_post_start_auth, post_null_t, post_null_tmpl_desc, {
                    })

DEFUN_HTTP_POST_API(h_as_post_send_msg, post_as_send_msg_t, post_as_send_msg_tmpl_desc, {
                    if (send_user_data(dstruct.msg->ptr, dstruct.msg->len, dstruct.sac) == LD_OK) {
                    HTTP_SEND_OK;
                    }else {
                    HTTP_SEND_FAILED;
                    }
                    }
)

DEFUN_HTTP_POST_API(h_gs_post_update_key, post_gs_key_upd_t, post_gs_key_upd_tmpl_desc, {
                    if (rcu_update_key(dstruct.sac)) {
                    HTTP_SEND_OK;
                    }else {
                    HTTP_SEND_FAILED;
                    }
                    }

)


// HTTP请求处理函数
static void h_g_post_create_sse(struct evhttp_request *req, void *arg) {
    http_server.sse_req = req;
    struct evkeyvalq *headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(headers, "Content-Type", "text/event-stream");
    evhttp_add_header(headers, "Cache-Control", "no-cache");
    evhttp_add_header(headers, "Connection", "keep-alive");
    evhttp_add_header(headers, "Access-Control-Allow-Origin", "*");

    // 发送初始化消息
    evhttp_send_reply_start(req, HTTP_OK, "");
}


const ld_http_ctrl_t controllers[] = {
    {
        "as/", NULL, {}, (ld_http_ctrl_t[]){
            {"testget", h_as_get_test, {}, NULL},
            {"testpost", h_as_post_test, {}, NULL},
            {"startauth", h_as_post_start_auth, {}, NULL},
            {"sendmsg", h_as_post_send_msg, {}, NULL},
            // {"pcap", h_as_get_pcap, {}, NULL},
            {NULL, NULL, NULL, NULL},
        }
    },
    {
        "gs/", NULL, {}, (ld_http_ctrl_t[]){
            {"updatekey", h_gs_post_update_key, {}, NULL},
            {NULL, NULL, NULL, NULL},
        }
    },
    {"occupyrcu", h_g_post_occupy_rcu, {}, NULL},
    {"opendevice", h_g_post_open_device, {}, NULL},
    {"getrcustate", h_g_get_rcu_state, {}, NULL},
    {"rcustop", h_g_post_rcu_stop, {}, NULL},
    {"createsse", h_g_post_create_sse, {}, NULL},
    {NULL, NULL, NULL, NULL},
};


static l_err init_http_path_ctrls(struct evhttp *hentity, const ld_http_ctrl_t *ctrls, const char *baseuri) {
    if (hentity == NULL) return LD_ERR_INVALID;

    for (int i = 0; ctrls[i].uri != NULL; i++) {
        char current_uri[URI_MAX_LEN] = {0};
        memcpy(current_uri, baseuri, strlen(baseuri));
        memcpy(current_uri + strlen(current_uri), ctrls[i].uri, strlen(ctrls[i].uri));

        evhttp_set_cb(hentity, current_uri, ctrls[i].controller, (void *) &ctrls->config);
        if (ctrls[i].children) {
            if (init_http_path_ctrls(hentity, ctrls[i].children, current_uri)) {
                return LD_ERR_INTERNAL;
            }
        }
    }
    return LD_OK;
}

l_err init_http_controllers(struct evhttp *hentity) {
    if (hentity == NULL) return LD_ERR_INVALID;
    init_http_path_ctrls(hentity, controllers, "/");
    evhttp_set_gencb(hentity, http_source_req_cb, NULL);

    return LD_OK;
}

static void http_source_req_cb(struct evhttp_request *req, void *arg) {
    evhttp_send_error(req, HTTP_BADREQUEST, "WRONG URL");
}

static l_err parse_get_query(struct evhttp_request *req, void *qstruct) {
    do {
        struct evhttp_uri *decoded = NULL;
        char *query = NULL;
        const char *uri = evhttp_request_get_uri(req); //获取请求uri

        if (uri == NULL) {
            log_error("NO URI");
            break;
        }

        //解码uri
        decoded = evhttp_uri_parse(uri);
        if (decoded == NULL) {
            log_warn("Not a good URI. Sending BADREQUEST");
            break;
        }

        //获取uri中的path部分
        const char *path = evhttp_uri_get_path(decoded);
        if (path == NULL) {
            log_error("WRONG PATH");
            break;
        }

        //获取uri中的参数部分
        query = (char *) evhttp_uri_get_query(decoded);
        if (query == NULL) {
            log_error("NO QUERY");
            break;
        }

        struct evkeyvalq params;
        //查询指定参数的值
        evhttp_parse_query_str(query, &params);
        int i = 0;
        for (struct evkeyval *p = params.tqh_first; p != NULL; p = p->next.tqe_next, i++) {
            memcpy(qstruct + BITS_PER_BYTE * i, &p->value, sizeof(void *));
        }
        return LD_OK;
    } while (0);

    evhttp_send_error(req, HTTP_BADREQUEST, "PPP");
    return LD_ERR_WRONG_QUERY;
}

static l_err parse_post_data(struct evhttp_request *req, void *struct_ptr, json_tmpl_desc_t *desc) {
    if (struct_ptr == NULL || desc == NULL) return LD_ERR_WRONG_PARA;

    struct evbuffer *inbuf = evhttp_request_get_input_buffer(req);
    char buf[1024] = {0};

    //逻辑有问题
    while (evbuffer_get_length(inbuf)) {
        if (evbuffer_remove(inbuf, buf, sizeof(buf) - 1) < 0) {
            evhttp_send_error(req, HTTP_BADREQUEST, "BAD REQUEST BUFFER");
            return LD_ERR_WRONG_DATA;
        }
    }

    cJSON *root = cJSON_Parse(buf);
    unmarshel_json(root, struct_ptr, desc);

    cJSON_Delete(root);
    return LD_OK;
}

