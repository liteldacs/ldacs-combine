//
// Created by jiaxv on 23-7-9.
//

#ifndef TEST_CLIENT_CLIENT_H
#define TEST_CLIENT_CLIENT_H
#include <ldacs_sim.h>

# define DEFAULT_FD -1

struct role_propt {
    sock_roles s_r;

    int (*server_make)(uint16_t port);

    int (*init_handler)(basic_conn_t *);
};


typedef struct net_opt_s {
    char name[32];
    // ldacs_roles role;
    sock_roles s_r;
    int server_fd; //for GSW
    char addr[GEN_ADDRLEN];
    int port;
    int timeout;

    void (*close_handler)(basic_conn_t *);

    bool (*reset_conn)(basic_conn_t *);

    l_err (*recv_handler)(basic_conn_t *);

    l_err (*send_handler)(basic_conn_t *);

    l_err (*accept_handler)(struct net_opt_s *);
} net_opt_t;


const struct role_propt *get_role_propt(int s_r);

void server_entity_setup(uint16_t port, net_opt_t *opt);


int server_shutdown(int server_fd);


int read_first_packet(basic_conn_t *bc, int pre_fd);


int first_request_handle(basic_conn_t *bc, int pre_fd);

extern int request_handle(basic_conn_t *bc);

extern int response_handle(basic_conn_t *bc);

int request_forward(basic_conn_t *bcp);

#endif //TEST_CLIENT_CLIENT_H
