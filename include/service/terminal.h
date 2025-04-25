//
// Created by 邹嘉旭 on 2024/6/10.
//

#ifndef TERMINAL_H
#define TERMINAL_H

#include <ldacs_sim.h>
#include <layer_rcu.h>
#include <layer_interface.h>

typedef struct terminal_obj_s {
    uint16_t AS_SAC;
} terminal_obj_t;

extern ld_service_t terminal_service;

typedef void (*terminal_func)(int argc, char **argv);


#endif //TERMINAL_H
