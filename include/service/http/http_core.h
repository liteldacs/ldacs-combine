//
// Created by 邹嘉旭 on 2024/4/24.
//

#ifndef HTTP_CORE_H
#define HTTP_CORE_H

#include <ldacs_sim.h>
#include <ldacs_utils.h>
#include "layer_rcu.h"

#define URI_MAX_LEN 512
#define SUFFIX 8

typedef void (*h_controller)(struct evhttp_request *request, void *arg);


extern ld_service_t http_service;

typedef struct http_ctrl_config_s {
    const char *res_name;
} http_ctrl_config_t;

typedef struct ld_http_ctrl_s {
    const char *uri;
    h_controller controller;
    http_ctrl_config_t config;
    struct ld_http_ctrl_s *children;
} ld_http_ctrl_t;

#include "http_server.h"
#include "http_request.h"
#include "http_response.h"

#endif //HTTP_CORE_H
