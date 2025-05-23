#include QMK_KEYBOARD_H

enum custom_keycodes {
    DF0_METASP = SAFE_RANGE,
    DF5_METASP,
    DF6_METASP
};
#define HOT_1 1
#define HOT_5 2*2*2*2*2
#define HOT_6 2*2*2*2*2*2
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case DF0_METASP:
            if (record->event.pressed) {
                if (default_layer_state != HOT_1 && default_layer_state != HOT_5) {
                    register_code(KC_LEFT_GUI);
                    register_code(KC_SPC);
                    unregister_code(KC_LEFT_GUI);
                    unregister_code(KC_SPC);
                }
                set_single_default_layer(0);
            } else {
                unregister_code(KC_LEFT_GUI);
                unregister_code(KC_SPC);
            }
            return false;
        case DF5_METASP:
            if (record->event.pressed) {
                set_single_default_layer(5);
            }
            return false;
        case DF6_METASP:
            if (record->event.pressed) {
                if (default_layer_state != HOT_6 && default_layer_state != HOT_5) {
                    register_code(KC_LEFT_GUI);
                    register_code(KC_SPC);
                    unregister_code(KC_LEFT_GUI);
                    unregister_code(KC_SPC);
                }
                set_single_default_layer(6);
            } else {
                unregister_code(KC_LEFT_GUI);
                unregister_code(KC_SPC);
            }
            return false;
    }
    return true;
}

enum td_keycodes {
    MO1_LGUI,
    MO8_LGUI
};
typedef enum {
    TD_NONE,
    TD_UNKNOWN,
    TD_SINGLE_TAP,
    TD_SINGLE_HOLD,
    TD_DOUBLE_TAP
} td_state_t;
typedef struct {
    bool is_press_action;
    td_state_t state;
} td_tap_t;
static td_tap_t mo1_lgui_tap_state = {
    .is_press_action = true,
    .state = TD_NONE
};
static td_tap_t mo8_lgui_tap_state = {
    .is_press_action = true,
    .state = TD_NONE
};

td_state_t cur_dance(tap_dance_state_t *state) {
    if (state->count == 1) {
        if (!state->pressed) return TD_SINGLE_TAP;
        else return TD_SINGLE_HOLD;
    } else if (state->count == 2) return TD_DOUBLE_TAP;
    else return TD_UNKNOWN;
}
void mo1_lgui_finished(tap_dance_state_t *state, void *user_data) {
    mo1_lgui_tap_state.state = cur_dance(state);
    switch (mo1_lgui_tap_state.state) {
        case TD_SINGLE_TAP:
            layer_on(1);
            break;
        case TD_SINGLE_HOLD:
            layer_on(1);
            break;
        case TD_DOUBLE_TAP:
            register_code(KC_LGUI);
            break;
        default:
            break;
    }
}
void mo8_lgui_finished(tap_dance_state_t *state, void *user_data) {
    mo8_lgui_tap_state.state = cur_dance(state);
    switch (mo8_lgui_tap_state.state) {
        case TD_SINGLE_TAP:
            layer_on(8);
            break;
        case TD_SINGLE_HOLD:
            layer_on(8);
            break;
        case TD_DOUBLE_TAP:
            register_code(KC_LGUI);
            break;
        default:
            break;
    }
}
void mo1_lgui_reset(tap_dance_state_t *state, void *user_data) {
    switch (mo1_lgui_tap_state.state) {
        case TD_SINGLE_TAP:
            layer_off(1);
            break;
        case TD_SINGLE_HOLD:
            layer_off(1);
            break;
        case TD_DOUBLE_TAP:
            unregister_code(KC_LGUI);
            break;
        default:
            break;
    }
    mo1_lgui_tap_state.state = TD_NONE;
}
void mo8_lgui_reset(tap_dance_state_t *state, void *user_data) {
    switch (mo8_lgui_tap_state.state) {
        case TD_SINGLE_TAP:
            layer_off(8);
            break;
        case TD_SINGLE_HOLD:
            layer_off(8);
            break;
        case TD_DOUBLE_TAP:
            unregister_code(KC_LGUI);
            break;
        default:
            break;
    }
    mo1_lgui_tap_state.state = TD_NONE;
}
uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case QK_TAP_DANCE ... QK_TAP_DANCE_MAX:
            return 275;
        default:
            return TAPPING_TERM;
    }
}

tap_dance_action_t tap_dance_actions[] = {
    [MO1_LGUI] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, mo1_lgui_finished, mo1_lgui_reset),
    [MO8_LGUI] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, mo8_lgui_finished, mo8_lgui_reset)
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_split_3x6_3(
        KC_TAB,  KC_F, KC_P, KC_D,    KC_L,           KC_X,    /**/  RCTL_T(KC_ENT),  KC_U,           KC_O,    KC_Y,   KC_B,    KC_Z,
        KC_ESC,  KC_S, KC_N, KC_T,    KC_H,           KC_K,    /**/  LCTL(KC_BSPC),   KC_A,           KC_E,    KC_I,   KC_C,    KC_Q,
        KC_LSFT, KC_V, KC_W, KC_G,    KC_M,           KC_J,    /**/  RALT_T(KC_COMM), RSFT_T(KC_DOT), KC_QUOT, KC_EQL, KC_SCLN, KC_SLSH,
                             KC_LALT, TD(MO1_LGUI), KC_LCTL,   /**/  KC_R,            LT(2,KC_SPC),   OSL(10)
    ),
    [1] = LAYOUT_split_3x6_3(
        LALT(KC_F4), KC_MUTE, KC_VOLD, KC_VOLU, KC_RGHT,       LCTL(KC_RGHT), /**/ LSFT(KC_ENT),  LSFT(KC_U), LSFT(KC_O),    LSFT(KC_Y), LSFT(KC_B),    LSFT(KC_Z),
        KC_ESC,      KC_PSCR, KC_HOME, KC_END,  KC_LEFT,       KC_UP,         /**/ LSFT(KC_BSPC), LSFT(KC_A), LSFT(KC_E),    LSFT(KC_I), LSFT(KC_C),    LSFT(KC_Q),
        KC_NO,       KC_MPLY, KC_MPRV, KC_MNXT, LCTL(KC_LEFT), KC_DOWN,       /**/ KC_NO,         KC_NO,      LSFT(KC_QUOT), KC_NO,      LSFT(KC_SCLN), LSFT(KC_SLSH),
                                       MO(4),   KC_TRNS,       MO(3),         /**/ LSFT(KC_R),    KC_LGUI,    KC_NO
    ),
    [2] = LAYOUT_split_3x6_3(
        RSFT(KC_TAB), RSFT(KC_F), RSFT(KC_P), RSFT(KC_D), RSFT(KC_L), RSFT(KC_X), /**/ KC_NO, KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO,
        RSFT(KC_ESC), RSFT(KC_S), RSFT(KC_N), RSFT(KC_T), RSFT(KC_H), RSFT(KC_K), /**/ KC_NO, KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO,
        KC_NO,        RSFT(KC_V), RSFT(KC_W), RSFT(KC_G), RSFT(KC_M), RSFT(KC_J), /**/ KC_NO, KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO,
                                              KC_LALT,    KC_LGUI,    KC_LCTL,    /**/ KC_NO, KC_TRNS, MO(13)
    ),
    [3] = LAYOUT_split_3x6_3(
        LGUI(KC_TAB), LGUI(KC_F), LGUI(KC_P), LGUI(KC_D), LGUI(KC_L), LGUI(KC_X), /**/ LGUI(KC_ENT),  LGUI(KC_U),   LGUI(KC_O),    LGUI(KC_Y),   LGUI(KC_B),    LGUI(KC_Z),
        LGUI(KC_ESC), LGUI(KC_S), LGUI(KC_N), LGUI(KC_T), LGUI(KC_H), LGUI(KC_K), /**/ LGUI(KC_BSPC), LGUI(KC_A),   LGUI(KC_E),    LGUI(KC_I),   LGUI(KC_C),    LGUI(KC_Q),
        KC_NO,        LGUI(KC_V), LGUI(KC_W), LGUI(KC_G), LGUI(KC_M), LGUI(KC_J), /**/ LGUI(KC_COMM), LGUI(KC_DOT), LGUI(KC_QUOT), LGUI(KC_EQL), LGUI(KC_SCLN), LGUI(KC_SLSH),
                                              KC_NO,      KC_TRNS,    KC_TRNS,    /**/ LGUI(KC_R),    KC_RSFT,      KC_RCTL
    ),
    [4] = LAYOUT_split_3x6_3(
        LAG(KC_TAB), LAG(KC_F), LAG(KC_P), LAG(KC_D), LAG(KC_L), LAG(KC_X), /**/  LAG(KC_ENT),  LAG(KC_U),   LAG(KC_O),    LAG(KC_Y),   LAG(KC_B),    LAG(KC_Z),
        LAG(KC_ESC), LAG(KC_S), LAG(KC_N), LAG(KC_T), LAG(KC_H), LAG(KC_K), /**/  LAG(KC_BSPC), LAG(KC_A),   LAG(KC_E),    LAG(KC_I),   LAG(KC_C),    LAG(KC_Q),
        KC_NO,       LAG(KC_V), LAG(KC_W), LAG(KC_G), LAG(KC_M), LAG(KC_J), /**/  LAG(KC_COMM), LAG(KC_DOT), LAG(KC_QUOT), LAG(KC_EQL), LAG(KC_SCLN), LAG(KC_SLSH),
                                           KC_TRNS,   KC_TRNS,    KC_NO,    /**/  LAG(KC_R),    KC_RSFT,     KC_RCTL
    ),
    [5] = LAYOUT_split_3x6_3(
        LSFT_T(KC_TAB), KC_Q, KC_W, KC_E,    KC_R,            KC_T,    /**/ KC_Y,    KC_U,           KC_I,    KC_O,   KC_P,    KC_BSPC,
        KC_ESC,         KC_A, KC_S, KC_D,    KC_F,            KC_G,    /**/ KC_H,    KC_J,           KC_K,    KC_L,   KC_SCLN, KC_QUOT,
        KC_RBRC,        KC_Z, KC_X, KC_C,    KC_V,            KC_B,    /**/ KC_N,    KC_M,           KC_COMM, KC_DOT, KC_SLSH, RCTL_T(KC_ENT),
                                    KC_LALT, LSFT_T(KC_LGUI), KC_LCTL, /**/ KC_LBRC, RSFT_T(KC_SPC), OSL(10)
    ),
    [6] = LAYOUT_split_3x6_3(
        KC_TAB,  KC_LBRC, KC_U, KC_V,    KC_W,         KC_I,    /**/ RCTL_T(KC_ENT), KC_E,            KC_J,   KC_B, KC_Q,    KC_SCLN,
        KC_ESC,  KC_K,    KC_Y, KC_R,    KC_N,         KC_L,    /**/ LCTL(KC_BSPC),  KC_F,            KC_T,   KC_S, KC_C,    KC_M,
        KC_LSFT, KC_X,    KC_G, KC_D,    KC_P,         KC_COMM, /**/ KC_QUES,        RSFT_T(KC_SLSH), KC_GRV, KC_Z, KC_RBRC, KC_A,
                                KC_LALT, TD(MO8_LGUI), KC_LCTL, /**/ KC_H,           LT(9,KC_SPC),    OSL(10)
    ),
    [7] = LAYOUT_split_3x6_3(
        KC_NO, KC_NO, KC_NO, MS_WHLU, MS_RGHT, MS_BTN2, /**/ KC_NO,   KC_NO,  KC_NO, KC_NO, KC_NO, KC_NO,
        KC_NO, KC_NO, KC_NO, MS_BTN3, MS_LEFT, MS_UP,   /**/ KC_NO,   KC_NO,  KC_NO, KC_NO, KC_NO, KC_NO,
        KC_NO, KC_NO, KC_NO, MS_WHLD, MS_BTN1, MS_DOWN, /**/ KC_NO,   KC_NO,  KC_NO, KC_NO, KC_NO, KC_NO,
                             KC_NO,   KC_NO,   KC_NO,   /**/ KC_TRNS, KC_SPC, TG(7)
    ),
    [8] = LAYOUT_split_3x6_3(
        LALT(KC_F4), KC_MUTE, KC_VOLD,  KC_VOLU, KC_RGHT,       LCTL(KC_RGHT), /**/ LSFT(KC_ENT),  LSFT(KC_E), LSFT(KC_J), LSFT(KC_B), LSFT(KC_Q), LSFT(KC_SCLN),
        KC_ESC,      KC_PSCR, KC_HOME,  KC_END,  KC_LEFT,       KC_UP,         /**/ LSFT(KC_BSPC), LSFT(KC_F), LSFT(KC_T), LSFT(KC_S), LSFT(KC_C), LSFT(KC_M),
        KC_NO,       KC_MPLY, KC_MPRV,  KC_MNXT, LCTL(KC_LEFT), KC_DOWN,       /**/ KC_NO,         KC_NO,      LSFT(KC_2), LSFT(KC_Z), LSFT(KC_RBRC), LSFT(KC_A),
                                        MO(12),  KC_TRNS,       MO(11),        /**/ LSFT(KC_H),    KC_LGUI,    KC_NO
    ),
    [9] = LAYOUT_split_3x6_3(
        LSFT(KC_TAB),  LSFT(KC_LBRC), LSFT(KC_U), LSFT(KC_V), LSFT(KC_W), LSFT(KC_I),    /**/ KC_NO,      LSFT(KC_QUOT),  LSFT(KC_BSLS), LSFT(KC_O), LSFT(KC_DOT), KC_NO,
        LSFT(KC_ESC),  LSFT(KC_K),    LSFT(KC_Y), LSFT(KC_R), LSFT(KC_N), LSFT(KC_L),    /**/ KC_NO,      KC_QUOT,        KC_BSLS,       KC_O,       KC_DOT,       KC_AMPR,
        KC_NO,         LSFT(KC_X),    LSFT(KC_G), LSFT(KC_D), LSFT(KC_P), LSFT(KC_COMM), /**/ KC_NO,      KC_CIRC,        KC_NO,         KC_EQL,     KC_DLR,       RALT(KC_SLSH),
                                                  KC_LALT,    KC_LGUI,    KC_LCTL,       /**/ LSFT(KC_H), KC_TRNS,        MO(13)
    ),
    [10] = LAYOUT_split_3x6_3(
        KC_GRV,  KC_ASTR, KC_UNDS, KC_LPRN, KC_RPRN, KC_BSLS,      /**/ KC_AMPR, KC_7,    KC_8,    KC_9,    KC_HASH, KC_EXLM,
        KC_CIRC, KC_PLUS, KC_MINS, KC_LCBR, KC_RCBR, LCTL(KC_DEL), /**/ KC_BSPC, KC_1,    KC_2,    KC_3,    KC_0,    KC_PIPE,
        KC_TILD, KC_LT,   KC_GT,   KC_LBRC, KC_RBRC, KC_DEL,       /**/ KC_DLR,  KC_4,    KC_5,    KC_6,    KC_PERC, KC_AT,
                                   KC_LALT, KC_LGUI, KC_LCTL,      /**/ TO(7),   MO(13),  KC_TRNS
    ),
    [11] = LAYOUT_split_3x6_3(
        LGUI(KC_TAB), LGUI(KC_SCLN), LGUI(KC_G), LGUI(KC_L),    LGUI(KC_K), LGUI(KC_W),    /**/ LGUI(KC_ENT),  LGUI(KC_E),    LGUI(KC_J), LGUI(KC_M),    LGUI(KC_DOT),  LGUI(KC_X),
        LGUI(KC_ESC), LGUI(KC_C),    LGUI(KC_Y), LGUI(KC_N),    LGUI(KC_D), LGUI(KC_COMM), /**/ LGUI(KC_BSPC), LGUI(KC_F),    LGUI(KC_T), LGUI(KC_S),    LGUI(KC_B),    LGUI(KC_I),
        KC_NO,        LGUI(KC_P),    LGUI(KC_R), LGUI(KC_LBRC), LGUI(KC_V), LGUI(KC_U),    /**/ LGUI(KC_QUES), LGUI(KC_SLSH), LGUI(KC_Z), LGUI(KC_QUOT), LGUI(KC_RBRC), LGUI(KC_Q),
                                                 KC_NO,         KC_TRNS,    KC_TRNS,       /**/ LGUI(KC_H),    KC_RSFT,       KC_NO
    ),
    [12] = LAYOUT_split_3x6_3(
        LAG(KC_TAB), LAG(KC_SCLN), LAG(KC_G), LAG(KC_L),    LAG(KC_K), LAG(KC_W), /**/ LAG(KC_ENT),  LAG(KC_E),    LAG(KC_J), LAG(KC_M),    LAG(KC_DOT),  LAG(KC_X),
        LAG(KC_ESC), LAG(KC_C),    LAG(KC_Y), LAG(KC_N),    LAG(KC_D), LAG(KC_V), /**/ LAG(KC_BSPC), LAG(KC_F),    LAG(KC_T), LAG(KC_S),    LAG(KC_B),    LAG(KC_I),
        KC_NO,       LAG(KC_P),    LAG(KC_R), LAG(KC_LBRC), LAG(KC_V), LAG(KC_U), /**/ LAG(KC_QUES), LAG(KC_SLSH), LAG(KC_Z), LAG(KC_QUOT), LAG(KC_RBRC), LAG(KC_Q),
                                              KC_TRNS,      KC_TRNS,   KC_NO,     /**/ LAG(KC_F),    KC_RSFT,      KC_NO
    ),
    [13] = LAYOUT_split_3x6_3(
        QK_BOOT, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, /**/ DF5_METASP, KC_NO,   KC_NO,  KC_NO,        KC_NO, KC_NO,
        KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, /**/ KC_NO, DF0_METASP,   DF6_METASP,  LGUI(KC_SPC), KC_NO, KC_NO,
        KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, /**/ KC_NO, KC_NO,   KC_NO,  KC_NO,        KC_NO, KC_NO,
                               KC_NO, KC_NO, KC_NO, /**/ KC_NO, KC_TRNS, KC_TRNS
    )
};


#ifdef OTHER_KEYMAP_C
#    include OTHER_KEYMAP_C
#endif // OoHER_KEYMAP_C
