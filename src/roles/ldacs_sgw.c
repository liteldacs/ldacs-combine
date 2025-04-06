//
// Created by 邹嘉旭 on 2024/11/9.
//
#include <gs_conn.h>
#include <ld_epoll.h>
#include "net_core/net.h"
#include "net_core/connection.h"
#include "ldacs_lme.h"

void run_sgw() {
    //TODO: 临时SGW存在问题，当前是一个GS<->SGW连接一个as_man，而未来要改成一个SAC一个as_man！！！！！！！1
    make_lme_layer();
}
