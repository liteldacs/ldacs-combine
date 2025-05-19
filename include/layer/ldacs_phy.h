//
// Created by 邹嘉旭 on 2023/12/8.
//

#ifndef LDACS_SIM_LDACS_PHY_H
#define LDACS_SIM_LDACS_PHY_H


#include "layer_core.h"
#include "ldacs_mac.h"

#define DEFAULT_SIGNAL_STRENGTH 0xF0
#define ATTACK_SIGNAL_STRENGTH 0xFF

enum PHY_SIM_LEVEL {
    PHY_SIM_JSON,
    PHY_SIM_REAL,
};

enum phy_timer_index {
    PHY_TIMER_BC = 0,
    PHY_TIMER_RA,
    PHY_TIMER_CC,
    PHY_TIMER_DC,
    PHY_TIMER_FL_DATA,
    PHY_TIMER_RL_DATA,
};

typedef enum {
    BCCH = 0,
    FL_CC_DCH,
    RACH,
    DCCH,
    RL_DCH,
} channels_en;

typedef struct phy_layer_objs_s phy_layer_objs_t;
typedef struct phy_sim_s phy_sim_t;
typedef struct cc_dch_merge_s cc_dch_merge_t;

struct phy_sim_s {
    enum PHY_SIM_LEVEL sim_level;

    l_err (*init_sim)(phy_layer_objs_t *obj);

    l_err (*upward_process)(void *data);

    l_err (*downward_process)(ld_prim_t *prim, buffer_t **in_bufs, buffer_t **out_buf);
};

struct cc_dch_merge_s {
    pthread_mutex_t mutex;
    bool has_cc;
    bool has_data;
    buffer_t *cc_buf[CC_BLK_N];
    buffer_t *data_bufs[FL_DATA_BLK_N];
    buffer_t *data_next_mf[2];
};

struct phy_layer_objs_s {
    uint8_t PHY_N_FFT; /* FFT size */
    uint32_t PHY_T_SA; /* Sampling Time, with nanosecond */
    double PHY_DEL_F; /* Sub-carrier spacing */

    phy_sim_t *sim;

    ld_gtimer_t phy_g_timer;
    gtimer_ev_t gtimer[10];

    cc_dch_merge_t cc_dch_merge;

    /* device */
    ld_dev_entity_t dev;
    pthread_t recv_th;
    ld_recv_args_t device_args;

    bool need_sync;

    pthread_t bc_start_th;
};

typedef struct p_rtx_obj_s {
    uint16_t rtx_type;
    pthread_t th;
} p_rtx_obj_t;

typedef enum {
    PHY_RA_REQ = 0x001,
    PHY_RA_IND = 0x002,
    PHY_RTX_RA_IND = 0x003,
    PHY_DC_REQ = 0x004,
    PHY_DC_IND = 0x005,
    PHY_RTX_DC_IND = 0x006,
    PHY_CC_REQ = 0x007,
    PHY_CC_IND = 0x008,
    PHY_RTX_CC_IND = 0x009,
    PHY_BC_REQ = 0x00A,
    PHY_BC_IND = 0x00B,
    PHY_RTX_BC_IND = 0x00C,
    PHY_DATA_REQ = 0x00D,
    PHY_DATA_IND = 0x00E,
    PHY_RTX_DATA_IND = 0x00F,
    PHY_FSCAN_REQ = 0x010,
    PHY_CSCAN_REQ = 0x011,
    PHY_GSCAN_REQ = 0x012,
    PHY_RDY_TO_SCAN_IND = 0x013,
    PHY_CONF_REQ = 0x014,
    PHY_CONF_IND = 0x015,
    PHY_FLSYNC_IND = 0x016,
    PHY_SYNC_REQ = 0x017,
    PHY_SYNC_IND = 0x018,
} ld_phy_prims_en;

#pragma pack(1)

typedef struct phy_json_hdr_s {
    uint8_t channel;
    // uint8_t sf_seq;
    uint8_t strength;
    char *raw;
} phy_json_hdr_t;

typedef struct bcch_pdu_s {
    buffer_t *bc_1;
    buffer_t *bc_2;
    buffer_t *bc_3;
} bcch_pdu_t;

typedef struct rach_pdu_s {
    buffer_t *ra_1;
    buffer_t *ra_2;
} rach_pdu_t;

typedef struct cc_dch_pdu_s {
    buffer_t *sdu_1_6;
    buffer_t *sdu_7_12;
    buffer_t *sdu_13_21;
    buffer_t *sdu_22_27;
} cc_dch_pdu_t;

typedef struct rl_dch_pdu_s {
    uint8_t start;
    uint8_t end;
    buffer_t *rl_dch;
} rl_dch_pdu_t;

typedef struct dcch_pdu_s {
    uint16_t slot_ser;
    buffer_t *dc;
} dcch_pdu_t;

#pragma pack()


extern json_tmpl_desc_t bcch_j_tmpl_desc;
extern json_tmpl_desc_t cc_dch_j_tmpl_desc;
extern json_tmpl_desc_t rach_j_tmpl_desc;
extern json_tmpl_desc_t dcch_j_tmpl_desc;
extern json_tmpl_desc_t rl_dch_j_tmpl_desc;
extern json_tmpl_desc_t phy_json_hdr_j_tmpl_desc;

extern phy_layer_objs_t phy_layer_objs;

l_err make_phy_layer(enum PHY_SIM_LEVEL level);

void P_SAPC(ld_prim_t *);

void P_SAPD(ld_prim_t *);

void P_SAPT(ld_prim_t *);

void P_SAPS(ld_prim_t *);

void process_phy_pkt(void *data);

l_err init_sim_json(phy_layer_objs_t *obj_p);

l_err init_sim_real(phy_layer_objs_t *obj_p);

l_err upward_json_sim(void *data);

// l_err downward_json_sim(ld_prim_t *prim, buffer_t **in_bufs, buffer_t *out_buf);
l_err downward_json_sim(ld_prim_t *prim, buffer_t **in_bufs, buffer_t **out_buf);

l_err upward_real(void *data);

l_err downward_real(ld_prim_t *prim, buffer_t **in_bufs, buffer_t **out_buf);

static void *trans_bc_timer_func(void *args);

static void *trans_cc_timer_func(void *args);

static void *trans_ra_timer_func(void *args);

static void *trans_dc_timer_func(void *args);

static void trans_fl_data_timer_func(void *args);

static void trans_rl_data_timer_func(void *args);

void as_init_gtimer();


extern ld_prim_t PHY_RA_REQ_PRIM;
extern ld_prim_t PHY_RA_IND_PRIM;
extern ld_prim_t PHY_RTX_RA_IND_PRIM;
extern ld_prim_t PHY_DC_REQ_PRIM;
extern ld_prim_t PHY_DC_IND_PRIM;
extern ld_prim_t PHY_RTX_DC_IND_PRIM;
extern ld_prim_t PHY_CC_REQ_PRIM;
extern ld_prim_t PHY_CC_IND_PRIM;
extern ld_prim_t PHY_RTX_CC_IND_PRIM;
extern ld_prim_t PHY_BC_REQ_PRIM;
extern ld_prim_t PHY_BC_IND_PRIM;
extern ld_prim_t PHY_RTX_BC_IND_PRIM;
extern ld_prim_t PHY_DATA_REQ_PRIM;
extern ld_prim_t PHY_DATA_IND_PRIM;
extern ld_prim_t PHY_RTX_DATA_IND_PRIM;
extern ld_prim_t PHY_FSCAN_REQ_PRIM;
extern ld_prim_t PHY_CSCAN_REQ_PRIM;
extern ld_prim_t PHY_GSCAN_REQ_PRIM;
extern ld_prim_t PHY_RDY_TO_SCAN_IND_PRIM;
extern ld_prim_t PHY_CONF_REQ_PRIM;
extern ld_prim_t PHY_CONF_IND_PRIM;
extern ld_prim_t PHY_FLSYNC_IND_PRIM;
extern ld_prim_t PHY_SYNC_REQ_PRIM;
extern ld_prim_t PHY_SYNC_IND_PRIM;

#endif //LDACS_SIM_LDACS_PHY_H
