//
// Created by 邹嘉旭 on 2024/3/21.
//

#ifndef LDACS_SIM_SECURE_CORE_H
#define LDACS_SIM_SECURE_CORE_H


#include <ldacs_sim.h>
#include <ldacs_utils.h>
#include <key_manage.h>

//改到crypto里
#include <gmssl/sm4.h>
#include <gmssl/sm3.h>
#include <gmssl/rand.h>


#define S_TYP_LEN 8
#define VER_LEN 3
#define PID_LEN 2
#define KEY_TYPE_LEN 4
#define NCC_LEN 16
#define NONCE_LEN 128 >> 3

#define AS_DB_NAME "/root/ldacs/combine/ldacs-combine/resources/as_sql.db"
#define GS_DB_NAME "/root/ldacs/combine/ldacs-combine/resources/gs_sql.db"
#define SGW_DB_NAME "/root/ldacs/combine/ldacs-combine/resources/sgw_sql.db"
#define KEY_BIN_PATH  "/root/ldacs/ldacs_sim_sgw/resources/keystore/rootkey.bin"
#define AS_KEY_TABLE "as_keystore"
#define GS_KEY_TABLE "gs_keystore"
#define SGW_KEY_TABLE "sgw_keystore"
#define ROOT_KEY_LEN 16
#define DEFAULT_VALIDATE 365


void generate_rand(uint8_t *rand, size_t len);

/* generate a rand int, max size is 64bits (8 bytes) */
uint64_t generate_urand(size_t rand_bits_sz);

/* generate a unlimit rand array */
void generate_nrand(uint8_t *rand, size_t sz);

#endif //LDACS_SIM_SECURE_CORE_H
