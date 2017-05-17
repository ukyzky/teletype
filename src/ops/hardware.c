#include "ops/hardware.h"

#include "helpers.h"
#include "ii.h"
#include "teletype_io.h"

static void op_CV_get(const void *data, scene_state_t *ss, exec_state_t *es,
                      command_state_t *cs);
static void op_CV_set(const void *data, scene_state_t *ss, exec_state_t *es,
                      command_state_t *cs);
static void op_CV_SLEW_get(const void *data, scene_state_t *ss,
                           exec_state_t *es, command_state_t *cs);
static void op_CV_SLEW_set(const void *data, scene_state_t *ss,
                           exec_state_t *es, command_state_t *cs);
static void op_CV_OFF_get(const void *data, scene_state_t *ss, exec_state_t *es,
                          command_state_t *cs);
static void op_CV_OFF_set(const void *data, scene_state_t *ss, exec_state_t *es,
                          command_state_t *cs);
static void op_IN_get(const void *NOTUSED(data), scene_state_t *ss,
                      exec_state_t *NOTUSED(es), command_state_t *cs);
static void op_PARAM_get(const void *NOTUSED(data), scene_state_t *ss,
                         exec_state_t *NOTUSED(es), command_state_t *cs);
static void op_TR_get(const void *data, scene_state_t *ss, exec_state_t *es,
                      command_state_t *cs);
static void op_TR_set(const void *data, scene_state_t *ss, exec_state_t *es,
                      command_state_t *cs);
static void op_TR_POL_get(const void *data, scene_state_t *ss, exec_state_t *es,
                          command_state_t *cs);
static void op_TR_POL_set(const void *data, scene_state_t *ss, exec_state_t *es,
                          command_state_t *cs);
static void op_TR_TIME_get(const void *data, scene_state_t *ss,
                           exec_state_t *es, command_state_t *cs);
static void op_TR_TIME_set(const void *data, scene_state_t *ss,
                           exec_state_t *es, command_state_t *cs);
static void op_TR_TOG_get(const void *data, scene_state_t *ss, exec_state_t *es,
                          command_state_t *cs);
static void op_TR_PULSE_get(const void *data, scene_state_t *ss,
                            exec_state_t *es, command_state_t *cs);
static void op_CV_SET_get(const void *data, scene_state_t *ss, exec_state_t *es,
                          command_state_t *cs);
static void op_MUTE_get(const void *data, scene_state_t *ss, exec_state_t *es,
                        command_state_t *cs);
static void op_MUTE_set(const void *data, scene_state_t *ss, exec_state_t *es,
                        command_state_t *cs);
static void op_STATE_get(const void *data, scene_state_t *ss, exec_state_t *es,
                         command_state_t *cs);


// clang-format off
const tele_op_t op_CV       = MAKE_GET_SET_OP(CV      , op_CV_get      , op_CV_set     , 1, true);
const tele_op_t op_CV_OFF   = MAKE_GET_SET_OP(CV.OFF  , op_CV_OFF_get  , op_CV_OFF_set , 1, true);
const tele_op_t op_CV_SLEW  = MAKE_GET_SET_OP(CV.SLEW , op_CV_SLEW_get , op_CV_SLEW_set, 1, true);
const tele_op_t op_IN       = MAKE_GET_OP    (IN      , op_IN_get      , 0, true);
const tele_op_t op_PARAM    = MAKE_GET_OP    (PARAM   , op_PARAM_get   , 0, true);
const tele_op_t op_PRM      = MAKE_ALIAS_OP  (PRM     , op_PARAM_get   , NULL,           0, true);
const tele_op_t op_TR       = MAKE_GET_SET_OP(TR      , op_TR_get      , op_TR_set     , 1, true);
const tele_op_t op_TR_POL   = MAKE_GET_SET_OP(TR.POL  , op_TR_POL_get  , op_TR_POL_set , 1, true);
const tele_op_t op_TR_TIME  = MAKE_GET_SET_OP(TR.TIME , op_TR_TIME_get , op_TR_TIME_set, 1, true);
const tele_op_t op_TR_TOG   = MAKE_GET_OP    (TR.TOG  , op_TR_TOG_get  , 1, false);
const tele_op_t op_TR_PULSE = MAKE_GET_OP    (TR.PULSE, op_TR_PULSE_get, 1, false);
const tele_op_t op_TR_P     = MAKE_ALIAS_OP  (TR.P    , op_TR_PULSE_get, NULL, 1, false);
const tele_op_t op_CV_SET   = MAKE_GET_OP    (CV.SET  , op_CV_SET_get  , 2, false);
const tele_op_t op_MUTE     = MAKE_GET_SET_OP(MUTE    , op_MUTE_get    , op_MUTE_set   , 1, true);
const tele_op_t op_STATE    = MAKE_GET_OP    (STATE   , op_STATE_get   , 1, true );
// clang-format on

static void op_CV_get(const void *NOTUSED(data), scene_state_t *ss,
                      exec_state_t *NOTUSED(es), command_state_t *cs) {
    int16_t a = cs_pop(cs);
    a--;
    if (a < 0)
        cs_push(cs, 0);
    else if (a < 4)
        cs_push(cs, ss->variables.cv[a]);
    else if (a < 20) {
        uint8_t d[] = { II_ANSIBLE_CV | II_GET, a & 0x3 };
        uint8_t addr = II_ANSIBLE_ADDR + (((a - 4) >> 2) << 1);
        tele_ii_tx(addr, d, 2);
        d[0] = 0;
        d[1] = 0;
        tele_ii_rx(addr, d, 2);
        cs_push(cs, (d[0] << 8) + d[1]);
    }
    else
        cs_push(cs, 0);
}

static void op_CV_set(const void *NOTUSED(data), scene_state_t *ss,
                      exec_state_t *NOTUSED(es), command_state_t *cs) {
    int16_t a = cs_pop(cs);
    int16_t b = cs_pop(cs);
    b = normalise_value(0, 16383, 0, b);
    a--;
    if (a < 0)
        return;
    else if (a < 4) {
        ss->variables.cv[a] = b;
        tele_cv(a, b, 1);
    }
    else if (a < 20) {
        uint8_t d[] = { II_ANSIBLE_CV, a & 0x3, b >> 8, b & 0xff };
        uint8_t addr = II_ANSIBLE_ADDR + (((a - 4) >> 2) << 1);

        tele_ii_tx(addr, d, 4);
    }
}

static void op_CV_SLEW_get(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
    int16_t a = cs_pop(cs);
    a--;
    if (a < 0)
        cs_push(cs, 0);
    else if (a < 4)
        cs_push(cs, ss->variables.cv_slew[a]);
    else if (a < 20) {
        uint8_t d[] = { II_ANSIBLE_CV_SLEW | II_GET, a & 0x3 };
        uint8_t addr = II_ANSIBLE_ADDR + (((a - 4) >> 2) << 1);
        tele_ii_tx(addr, d, 2);
        d[0] = 0;
        d[1] = 0;
        tele_ii_rx(addr, d, 2);
        cs_push(cs, (d[0] << 8) + d[1]);
    }
    else
        cs_push(cs, 0);
}

static void op_CV_SLEW_set(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
    int16_t a = cs_pop(cs);
    int16_t b = cs_pop(cs);
    b = normalise_value(1, 32767, 0, b);  // min slew = 1
    a--;
    if (a < 0)
        return;
    else if (a < 4) {
        ss->variables.cv_slew[a] = b;
        tele_cv_slew(a, b);
    }
    else if (a < 20) {
        uint8_t d[] = { II_ANSIBLE_CV_SLEW, a & 0x3, b >> 8, b & 0xff };
        uint8_t addr = II_ANSIBLE_ADDR + (((a - 4) >> 2) << 1);
        tele_ii_tx(addr, d, 4);
    }
}

static void op_CV_OFF_get(const void *NOTUSED(data), scene_state_t *ss,
                          exec_state_t *NOTUSED(es), command_state_t *cs) {
    int16_t a = cs_pop(cs);
    a--;
    if (a < 0)
        cs_push(cs, 0);
    else if (a < 4)
        cs_push(cs, ss->variables.cv_off[a]);
    else if (a < 20) {
        uint8_t d[] = { II_ANSIBLE_CV_OFF | II_GET, a & 0x3 };
        uint8_t addr = II_ANSIBLE_ADDR + (((a - 4) >> 2) << 1);
        tele_ii_tx(addr, d, 2);
        d[0] = 0;
        d[1] = 0;
        tele_ii_rx(addr, d, 2);
        cs_push(cs, (d[0] << 8) + d[1]);
    }
    else
        cs_push(cs, 0);
}

static void op_CV_OFF_set(const void *NOTUSED(data), scene_state_t *ss,
                          exec_state_t *NOTUSED(es), command_state_t *cs) {
    int16_t a = cs_pop(cs);
    int16_t b = cs_pop(cs);
    ss->variables.cv_off[a] = b;
    a--;
    if (a < 0)
        return;
    else if (a < 4) {
        ss->variables.cv_off[a] = b;
        tele_cv_off(a, b);
        tele_cv(a, ss->variables.cv[a], 1);
    }
    else if (a < 20) {
        uint8_t d[] = { II_ANSIBLE_CV_OFF, a & 0x3, b >> 8, b & 0xff };
        uint8_t addr = II_ANSIBLE_ADDR + (((a - 4) >> 2) << 1);
        tele_ii_tx(addr, d, 4);
    }
}

static void op_IN_get(const void *NOTUSED(data), scene_state_t *ss,
                      exec_state_t *NOTUSED(es), command_state_t *cs) {
    cs_push(cs, ss->variables.in);
}

static void op_PARAM_get(const void *NOTUSED(data), scene_state_t *ss,
                         exec_state_t *NOTUSED(es), command_state_t *cs) {
    cs_push(cs, ss->variables.param);
}

static void op_TR_get(const void *NOTUSED(data), scene_state_t *ss,
                      exec_state_t *NOTUSED(es), command_state_t *cs) {
    int16_t a = cs_pop(cs);
    a--;
    if (a < 0)
        cs_push(cs, 0);
    else if (a < 4)
        cs_push(cs, ss->variables.tr[a]);
    else if (a < 20) {
        uint8_t d[] = { II_ANSIBLE_TR | II_GET, a & 0x3 };
        uint8_t addr = II_ANSIBLE_ADDR + (((a - 4) >> 2) << 1);
        tele_ii_tx(addr, d, 2);
        d[0] = 0;
        tele_ii_rx(addr, d, 1);
        cs_push(cs, d[0]);
    }
    else
        cs_push(cs, 0);
}

static void op_TR_set(const void *NOTUSED(data), scene_state_t *ss,
                      exec_state_t *NOTUSED(es), command_state_t *cs) {
    int16_t a = cs_pop(cs);
    int16_t b = cs_pop(cs);
    a--;
    if (a < 0)
        return;
    else if (a < 4) {
        ss->variables.tr[a] = b != 0;
        tele_tr(a, b);
    }
    else if (a < 20) {
        uint8_t d[] = { II_ANSIBLE_TR, a & 0x3, b };
        uint8_t addr = II_ANSIBLE_ADDR + (((a - 4) >> 2) << 1);
        tele_ii_tx(addr, d, 3);
    }
}

static void op_TR_POL_get(const void *NOTUSED(data), scene_state_t *ss,
                          exec_state_t *NOTUSED(es), command_state_t *cs) {
    int16_t a = cs_pop(cs);
    a--;
    if (a < 0)
        cs_push(cs, 0);
    else if (a < 4)
        cs_push(cs, ss->variables.tr_pol[a]);
    else if (a < 20) {
        uint8_t d[] = { II_ANSIBLE_TR_POL | II_GET, a & 0x3 };
        uint8_t addr = II_ANSIBLE_ADDR + (((a - 4) >> 2) << 1);
        tele_ii_tx(addr, d, 2);
        d[0] = 0;
        tele_ii_rx(addr, d, 1);
        cs_push(cs, d[0]);
    }
    else
        cs_push(cs, 0);
}

static void op_TR_POL_set(const void *NOTUSED(data), scene_state_t *ss,
                          exec_state_t *NOTUSED(es), command_state_t *cs) {
    int16_t a = cs_pop(cs);
    int16_t b = cs_pop(cs);
    a--;
    if (a < 0)
        return;
    else if (a < 4) {
        ss->variables.tr_pol[a] = b > 0;
    }
    else if (a < 20) {
        uint8_t d[] = { II_ANSIBLE_TR_POL, a & 0x3, b > 0 };
        uint8_t addr = II_ANSIBLE_ADDR + (((a - 4) >> 2) << 1);
        tele_ii_tx(addr, d, 3);
    }
}

static void op_TR_TIME_get(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
    int16_t a = cs_pop(cs);
    a--;
    if (a < 0)
        cs_push(cs, 0);
    else if (a < 4)
        cs_push(cs, ss->variables.tr_time[a]);
    else if (a < 20) {
        uint8_t d[] = { II_ANSIBLE_TR_TIME | II_GET, a & 0x3 };
        uint8_t addr = II_ANSIBLE_ADDR + (((a - 4) >> 2) << 1);
        tele_ii_tx(addr, d, 2);
        d[0] = 0;
        d[1] = 0;
        tele_ii_rx(addr, d, 2);
        cs_push(cs, (d[0] << 8) + d[1]);
    }
    else
        cs_push(cs, 0);
}

static void op_TR_TIME_set(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
    int16_t a = cs_pop(cs);
    int16_t b = cs_pop(cs);
    if (b < 0) b = 0;
    a--;
    if (a < 0)
        return;
    else if (a < 4) {
        ss->variables.tr_time[a] = b;
    }
    else if (a < 20) {
        uint8_t d[] = { II_ANSIBLE_TR_TIME, a & 0x3, b >> 8, b & 0xff };
        uint8_t addr = II_ANSIBLE_ADDR + (((a - 4) >> 2) << 1);
        tele_ii_tx(addr, d, 4);
    }
}

static void op_TR_TOG_get(const void *NOTUSED(data), scene_state_t *ss,
                          exec_state_t *NOTUSED(es), command_state_t *cs) {
    int16_t a = cs_pop(cs);
    a--;
    if (a < 0)
        return;
    else if (a < 4) {
        if (ss->variables.tr[a])
            ss->variables.tr[a] = 0;
        else
            ss->variables.tr[a] = 1;
        tele_tr(a, ss->variables.tr[a]);
    }
    else if (a < 20) {
        uint8_t d[] = { II_ANSIBLE_TR_TOG, a & 0x3 };
        uint8_t addr = II_ANSIBLE_ADDR + (((a - 4) >> 2) << 1);
        tele_ii_tx(addr, d, 2);
    }
}

static void op_TR_PULSE_get(const void *NOTUSED(data), scene_state_t *ss,
                            exec_state_t *NOTUSED(es), command_state_t *cs) {
    int16_t a = cs_pop(cs);
    a--;
    if (a < 0)
        return;
    else if (a < 4) {
        int16_t time = ss->variables.tr_time[a];  // pulse time
        if (time <= 0) return;  // if time <= 0 don't do anything
        ss->variables.tr[a] = ss->variables.tr_pol[a];
        ss->tr_pulse_timer[a] = time;  // set time
        tele_tr(a, ss->variables.tr[a]);
    }
    else if (a < 20) {
        uint8_t d[] = { II_ANSIBLE_TR_PULSE, a & 0x3 };
        uint8_t addr = II_ANSIBLE_ADDR + (((a - 4) >> 2) << 1);
        tele_ii_tx(addr, d, 2);
    }
}

static void op_CV_SET_get(const void *NOTUSED(data), scene_state_t *ss,
                          exec_state_t *NOTUSED(es), command_state_t *cs) {
    int16_t a = cs_pop(cs);
    int16_t b = cs_pop(cs);

    if (b < 0)
        b = 0;
    else if (b > 16383)
        b = 16383;

    a--;
    if (a < 0)
        return;
    else if (a < 4) {
        ss->variables.cv[a] = b;
        tele_cv(a, b, 0);
    }
    else if (a < 20) {
        uint8_t d[] = { II_ANSIBLE_CV_SET, a & 0x3, b >> 8, b & 0xff };
        uint8_t addr = II_ANSIBLE_ADDR + (((a - 4) >> 2) << 1);
        tele_ii_tx(addr, d, 4);
    }
}

static void op_MUTE_get(const void *NOTUSED(data), scene_state_t *ss,
                        exec_state_t *NOTUSED(es), command_state_t *cs) {
    int16_t a = cs_pop(cs) - 1;
    if (a >= 0 && a < TRIGGER_INPUTS) { cs_push(cs, ss_get_mute(ss, a)); }
    else {
        cs_push(cs, 0);
    }
}

static void op_MUTE_set(const void *NOTUSED(data), scene_state_t *ss,
                        exec_state_t *NOTUSED(es), command_state_t *cs) {
    int16_t a = cs_pop(cs) - 1;
    bool b = cs_pop(cs) > 0;
    if (a >= 0 && a < TRIGGER_INPUTS) { ss_set_mute(ss, a, b); }
}

static void op_STATE_get(const void *NOTUSED(data), scene_state_t *NOTUSED(ss),
                         exec_state_t *NOTUSED(es), command_state_t *cs) {
    int16_t a = cs_pop(cs);
    a--;
    if (a < 0)
        cs_push(cs, 0);
    else if (a < 8)
        cs_push(cs, tele_get_input_state(a));
    else if (a < 24) {
        uint8_t d[] = { II_ANSIBLE_INPUT | II_GET, a & 0x3 };
        uint8_t addr = II_ANSIBLE_ADDR + (((a - 8) >> 2) << 1);
        tele_ii_tx(addr, d, 2);
        d[0] = 0;
        tele_ii_rx(addr, d, 1);
        cs_push(cs, d[0]);
    }
    else
        cs_push(cs, 0);
}
