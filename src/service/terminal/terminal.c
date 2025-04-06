//
// Created by 邹嘉旭 on 2024/6/10.
//

#include "service/terminal.h"

static l_err init_terminal_service();

static void handle_st_chg_terminal(lme_state_chg_t *);

static void handle_as_info_key_upd_terminal(as_info_key_upd_t *as_upd);

static void handle_as_info_upd_terminal(as_info_upd_t *as_info);

static void handle_user_msg_terminal(user_msg_t *umsg);


ld_service_t terminal_service = {
    .init_service = init_terminal_service,
    .handle_state_chg = handle_st_chg_terminal,
    .handle_as_info_key_upd = handle_as_info_key_upd_terminal,
    .handle_as_info_upd = handle_as_info_upd_terminal,
    .handle_recv_user_msg = handle_user_msg_terminal,
};


static l_err init_terminal_service() {
    log_info("The ldacs simulator is using 'UNUSE HTTP' mode.");

    rcu_power_on(config.role); //power on directly
    if (config.role == LD_AS) rcu_start_auth();

    while (stop_flag == FALSE) {
        sleep(60 * 60 * 24);
    }
    return LD_OK;
}


static void handle_st_chg_terminal(lme_state_chg_t *st_chg) {
    // log_warn("The Current LME state is %d, by %.02x", st_chg->state, st_chg->sac);
}

static void handle_as_info_key_upd_terminal(as_info_key_upd_t *as_upd) {
    // log_warn("NEW value of AS key %s is %02x, by %.02x", as_upd->key->ptr, as_upd->value, as_upd->ua);
}

static void handle_as_info_upd_terminal(as_info_upd_t *as_info) {
    // log_fatal("NEW value of AS by %.02x", as_info->AS_UA);
}

static void handle_user_msg_terminal(user_msg_t *umsg) {
    log_fatal("USER MESSAGE %s", umsg->msg->ptr);
}
