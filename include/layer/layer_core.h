//
// Created by 邹嘉旭 on 2023/12/8.
//

#ifndef LDACS_SIM_LAYER_CORE_H
#define LDACS_SIM_LAYER_CORE_H

#include <ldacs_sim.h>
#include <ldacs_utils.h>
#include <device.h>
#include  <ld_newtimer.h>
#include <ld_santilizer.h>
#include <ld_primitive.h>

#define DCL_MAX 32
#define CO_MAX 1 << 9

#define BC_BLK_N 3
#define BC_BLK_LEN_1_3  528 >> 3  //66 Bytes
#define BC_BLK_LEN_2    1000 >> 3 //125 Bytes
#define BC_BLK_FORCE_REMAIN 272 >> 8 // 272 = 4 + 256 + 4 + 8

#define CC_BLK_N 1
#define CC_BLK_LEN_MAX  728         // 728b * 8(phy-pdus) / 8 == 728Bytes
#define CC_BLK_LEN_MIN  728 >> 3    // 728b / 8 == 91Bytes

#define RA_BLK_N 2
#define RA_BLK_LEN  ((54 >> 3) + 1)

#define DC_BLK_N 1
#define DC_SLOT_MAX (1 << 5)  //32 DC slots per MF
#define DC_SDU_LEN_MAX (75 >> 3)
#define DC_BLK_LEN_MAX  ((83 >> 3) + 1)

/* During the testing phase, all use QPSK with 1/2 cc */
#define FL_DATA_BLK_N 4
#define FL_DATA_BLK_2_F_LEN_MAX  546   // ( 728b / 8 ) * 6 == 546
#define FL_DATA_BLK_3_F_LEN_MAX 728    // ( 728b / 8 ) * (9 - 1) == 728

#define RL_DATA_BLK_N 1
#define RL_SLOT_MAX 160                 // (58.32/0.72) = (81(double slots) - 1(sync slot)) * 2 = 160(single slot)
#define RL_DATA_BLK_LEN_MAX 14   //(112b / 8)

#define CMS_1_PER_LEN 728 >> 3

enum ELE_TYP {
    ELE_TYP_RESERVED = 0x00,

    /* BC type */
    B_TYP_ACB = 0x1,
    B_TYP_SIB = 0x2,
    B_TYP_STB = 0X3,
    B_TYP_GS_POS_B = 0x5,
    B_TYP_BC_MAC = 0X6,
    B_TYP_SF_NUMBER = 0x7,


    /* CC type */
    C_TYP_RESERVED = 0x00,
    C_TYP_SLOT_DESC = 0x13, // ?
    C_TYP_DCCH_DESC = 0x01,
    C_TYP_CMS_FL = 0x02,
    C_TYP_DCCH_POLL = 0x03,
    C_TYP_CELL_RESP = 0x04,
    C_TYP_CHANGE_CO = 0x05,
    C_TYP_CELL_DENIED = 0x06,
    C_TYP_LM_DATA = 0x07,
    C_TYP_ACK = 0x09,
    C_TYP_ACK_FRAG = 0x0A,
    C_TYP_FL_ALLOC = 0x0B,
    C_TYP_RL_ALLOC = 0x0C,
    C_TYP_P_RL_ALLOC = 0x0D,
    C_TYP_SYNC_POLL = 0x0E,
    C_TYP_HO_COM = 0x0F,
    C_TYP_KEEY_ALIVE = 0x10,
    C_TYP_P_RL_CANCEL = 0x11,
    C_TYP_CC_MAC = 0x12,

    /* RA type */
    R_TYP_CR = 0x01,

    /* DC type */
    DC_TYP_PADDING = 0X0,
    DC_TYP_POW_REP = 0X1,
    DC_TYP_ACK = 0x3,
    DC_TYP_ACK_FRAG = 0x4,
    DC_TYP_CELL_EXIT = 0x5,
    DC_TYP_PRSC_RQST_CANCEL = 0x6,
    DC_TYP_RSC_RQST = 0x7,
    DC_TYP_PRSC_RQST = 0x8,
    DC_TYP_KEEP_ALIVE = 0x9,
    DC_TYP_VCS = 0xA,
    DC_TYP_VTN = 0xB,


    /* DCH type */
    D_TYP_FL = 0x1,
    D_TYP_RL = 0x2,

    /* LME */
    // L_TYP_ST_FSCANNING = 0x01,
    // L_TYP_ST_CSCANNING = 0x02,
    // L_TYP_ST_CONNECTING = 0x03,
    // L_TYP_ST_AUTH = 0x04,
    // L_TYP_ST_OPEN = 0x05,
    LME_STATE_CHANGE = 0x01,
    LME_AS_KEY_UPDATE = 0x02,
    LME_AS_UPDATE = 0x03,

    /* SN */
    SN_TYP_FROM_LME = 0x1,
    SN_TYP_FROM_UP = 0x2,

    /* DLS */
    DL_TYP_AS_INIT = 0x1,
    DL_TYP_GS_INIT = 0x2,

    /* RCU */
    RC_TYP_OPEN = 0x0,
    RC_TYP_CLOSE = 0x1,

    /* Verify */
    VER_PASS = 0xE0,
    VER_WRONG_MAC = 0xE1,
    VER_WRONG_SQN = 0xE2,

    /* else type */
    E_TYP_ANY = 0XF0,
    E_TYP_BUFFER = 0xF1,
    E_TYP_FL = 0xF2,
    E_TYP_RL = 0xF3,
    E_TYP_UNDEFINED = 0xFF,
};


enum TYPE_LEN {
    B_TYPE_LEN = 4, //  4 bits
    C_TYPE_LEN = 5, //  5 bits
    R_TYPE_LEN = 2, //  2 bits
    D_TYPE_LEN = 4, //  4 bits
};

enum ld_timev_idx {
    SF_TIMER_IDX = 0,
    MF_TIMER_IDX,
    BC_TIMER_INTVL_IDX,
    CC_TIMER_INTVL_IDX,
};

enum CHANNEL_E {
    BC_CHANNEL = 0,
    CC_CHANNEL,
    RA_CHANNEL,
    DC_CHANNEL,
    FL_CHANNEL,
    RL_CHANNEL,
};

enum FLOW_DIRECT {
    DIR_MAC = -1,
    DIR_LME = 0,
    DIR_DLS,
    DIR_VI,
};

typedef enum {
    DLS_COS_0 = 0X0,
    DLS_COS_1 = 0X1,
    DLS_COS_2 = 0X2,
    DLS_COS_3 = 0X3,
    DLS_COS_4 = 0X4,
    DLS_COS_5 = 0X5,
    DLS_COS_6 = 0X6,
    DLS_COS_7 = 0X7,
} DLS_COS;

typedef struct ld_format_desc_s {
    enum ELE_TYP type;
    struct_desc_t *f_desc;
    size_t desc_size;
    size_t struct_size;

    void (*free_func)(void *);

    pqueue_pri_t pri;
    enum FLOW_DIRECT direct;
} ld_format_desc_t;


typedef struct sdu_s_s {
    size_t blk_n;
    buffer_t **blks;
} sdu_s_t;

static inline sdu_s_t *create_sdu_s(size_t blk_n) {
    sdu_s_t *sdu_s_p = malloc(sizeof(sdu_s_t));
    sdu_s_p->blk_n = blk_n;
    sdu_s_p->blks = calloc(sdu_s_p->blk_n, sizeof(buffer_t *));
    return sdu_s_p;
}

static inline void free_sdu_s(void *p) {
    sdu_s_t *sdu_p = p;
    if (sdu_p) {
        if (sdu_p->blks) {
            FREE_BUF_ARRAY_DEEP2(sdu_p->blks, sdu_p->blk_n);
        }
        free(sdu_p);
    }
}

/* 具备指向性的sdu */
typedef struct orient_sdu_s {
    buffer_t *buf;
    uint16_t AS_SAC,
            GS_SAC;
} orient_sdu_t;

extern ld_gtimer_t gtimer;

static orient_sdu_t *create_orient_sdus(uint16_t AS_SAC, uint16_t GS_SAC) {
    orient_sdu_t *orient_sdu = malloc(sizeof(orient_sdu_t));
    orient_sdu->buf = init_buffer_unptr();
    orient_sdu->AS_SAC = AS_SAC;
    orient_sdu->GS_SAC = GS_SAC;
    return orient_sdu;
}

static void free_orient_sdus(void *p) {
    orient_sdu_t *orient_sdu_p = p;
    if (orient_sdu_p) {
        //free_buffer_v(&orient_sdu_p->buf);
        free_buffer(orient_sdu_p->buf);
        free(orient_sdu_p);
    }
}

typedef struct lyr_desc_s {
    const char *name;
    void *lyr_funcs;
    void *lyr_obj;
    struct lyr_desc_s **upper_lyr;
    struct lyr_desc_s **lower_lyr;
} lyr_desc_t;

typedef struct ld_queue_node_s {
    /* only for pqueue */
    pqueue_pri_t pri;
    size_t pos;

    enum ELE_TYP type;
    void *n_data;
    free_func free_func;
} ld_queue_node_t;

static ld_queue_node_t *init_queue_node(enum ELE_TYP type, void *ndata, free_func free_func) {
    ld_queue_node_t *q_node = calloc(1, sizeof(ld_queue_node_t));
    q_node->type = type;
    q_node->n_data = ndata;
    q_node->free_func = free_func;
    return q_node;
}

static void free_queue_node(void *n) {
    ld_queue_node_t *node = n;
    if (node) {
        node->free_func(node->n_data);
        free(node);
    }
}


static ld_queue_node_t *init_pqueue_node(pqueue_pri_t pri, enum ELE_TYP type,
                                         void *ndata, free_func free_func) {
    ld_queue_node_t *q_node = init_queue_node(type, ndata, free_func);
    q_node->pri = pri;
    q_node->pos = 0;
    return q_node;
}

static int ld_cmp_pri(pqueue_pri_t next, pqueue_pri_t curr) {
    return (next >= curr);
}

static pqueue_pri_t ld_get_pri(void *a) {
    return ((ld_queue_node_t *) a)->pri;
}

static void ld_set_pri(void *a, pqueue_pri_t pri) {
    ((ld_queue_node_t *) a)->pri = pri;
}

static size_t ld_get_pos(void *a) {
    return ((ld_queue_node_t *) a)->pos;
}

static void ld_set_pos(void *a, size_t pos) {
    ((ld_queue_node_t *) a)->pos = pos;
}

typedef struct fscanning_freqs_s {
    double avial_fl_freqs[256],
            avial_rl_freqs[256];
} fscanning_freqs_t;


#endif //LDACS_SIM_LAYER_CORE_H
