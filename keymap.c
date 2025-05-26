#include QMK_KEYBOARD_H

#define L_EN 0
#define L_EN_SPECIAL 1
#define L_EN_SH 2
#define L_QW 3
#define L_UA 4
#define L_UA_SPECIAL 5
#define L_UA_SH 6
#define L_CTL 7
#define L_ALT 8
#define L_LGUI 9
#define L_LAG 10
#define L_MOUSE 11
#define L_SYMBOL 12
#define L_SWITCH 13

#define HOT_EN 1
#define HOT_QW 2*2*2
#define HOT_UA 2*2*2*2

enum custom_keycodes {
    DF_EN_LGUISP = SAFE_RANGE,
    DF_QW_LGUISP,
    DF_UA_LGUISP
};
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case DF_EN_LGUISP:
            if (record->event.pressed) {
                if (default_layer_state != HOT_EN && default_layer_state != HOT_QW) {
                    register_code(KC_LEFT_GUI);
                    register_code(KC_SPC);
                    unregister_code(KC_LEFT_GUI);
                    unregister_code(KC_SPC);
                }
                set_single_default_layer(L_EN);
            } else {
                unregister_code(KC_LEFT_GUI);
                unregister_code(KC_SPC);
            }
            return false;
        case DF_QW_LGUISP:
            if (record->event.pressed) {
                set_single_default_layer(L_QW);
            }
            return false;
        case DF_UA_LGUISP:
            if (record->event.pressed) {
                if (default_layer_state != HOT_UA && default_layer_state != HOT_QW) {
                    register_code(KC_LEFT_GUI);
                    register_code(KC_SPC);
                    unregister_code(KC_LEFT_GUI);
                    unregister_code(KC_SPC);
                }
                set_single_default_layer(L_UA);
            } else {
                unregister_code(KC_LEFT_GUI);
                unregister_code(KC_SPC);
            }
            return false;
    }
    return true;
}

enum td_keycodes {
    MO_ENSPECIAL_LGUI,
    MO_UASPECIAL_LGUI
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
static td_tap_t mo_enlsh_lgui_tap_state = {
    .is_press_action = true,
    .state = TD_NONE
};
static td_tap_t mo_ualsh_lgui_tap_state = {
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
void mo_enlsh_lgui_finished(tap_dance_state_t *state, void *user_data) {
    mo_enlsh_lgui_tap_state.state = cur_dance(state);
    switch (mo_enlsh_lgui_tap_state.state) {
        case TD_SINGLE_TAP:
            layer_on(L_EN_SPECIAL);
            break;
        case TD_SINGLE_HOLD:
            layer_on(L_EN_SPECIAL);
            break;
        case TD_DOUBLE_TAP:
            register_code(KC_LGUI);
            break;
        default:
            break;
    }
}
void mo_ualsh_lgui_finished(tap_dance_state_t *state, void *user_data) {
    mo_ualsh_lgui_tap_state.state = cur_dance(state);
    switch (mo_ualsh_lgui_tap_state.state) {
        case TD_SINGLE_TAP:
            layer_on(L_UA_SPECIAL);
            break;
        case TD_SINGLE_HOLD:
            layer_on(L_UA_SPECIAL);
            break;
        case TD_DOUBLE_TAP:
            register_code(KC_LGUI);
            break;
        default:
            break;
    }
}
void mo_enlsh_lgui_reset(tap_dance_state_t *state, void *user_data) {
    switch (mo_enlsh_lgui_tap_state.state) {
        case TD_SINGLE_TAP:
            layer_off(L_EN_SPECIAL);
            break;
        case TD_SINGLE_HOLD:
            layer_off(L_EN_SPECIAL);
            break;
        case TD_DOUBLE_TAP:
            unregister_code(KC_LGUI);
            break;
        default:
            break;
    }
    mo_enlsh_lgui_tap_state.state = TD_NONE;
}
void mo_ualsh_lgui_reset(tap_dance_state_t *state, void *user_data) {
    switch (mo_ualsh_lgui_tap_state.state) {
        case TD_SINGLE_TAP:
            layer_off(L_UA_SPECIAL);
            break;
        case TD_SINGLE_HOLD:
            layer_off(L_UA_SPECIAL);
            break;
        case TD_DOUBLE_TAP:
            unregister_code(KC_LGUI);
            break;
        default:
            break;
    }
    mo_enlsh_lgui_tap_state.state = TD_NONE;
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
    [MO_ENSPECIAL_LGUI] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, mo_enlsh_lgui_finished, mo_enlsh_lgui_reset),
    [MO_UASPECIAL_LGUI] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, mo_ualsh_lgui_finished, mo_ualsh_lgui_reset)
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [L_EN] = LAYOUT_split_3x6_3(
        KC_TAB,  KC_F, KC_P, KC_D,    KC_L,                  KC_X,     /**/  RCTL_T(KC_ENT),  KC_U,                KC_O,    KC_Y,   KC_B,    KC_Z,
        KC_ESC,  KC_S, KC_N, KC_T,    KC_H,                  KC_K,     /**/  LCTL(KC_BSPC),   KC_A,                KC_E,    KC_I,   KC_C,    KC_Q,
        KC_LSFT, KC_V, KC_W, KC_G,    KC_M,                  KC_J,     /**/  RALT_T(KC_COMM), RSFT_T(KC_DOT),      KC_QUOT, KC_EQL, KC_SCLN, KC_SLSH,
                             KC_LALT, TD(MO_ENSPECIAL_LGUI), KC_LCTL,  /**/  KC_R,            LT(L_EN_SH,KC_SPC), OSL(L_SYMBOL)
    ),
    [L_EN_SPECIAL] = LAYOUT_split_3x6_3(
        LALT(KC_F4), KC_MUTE, KC_VOLD, KC_VOLU, KC_RGHT,       LCTL(KC_RGHT), /**/ KC_NO,      KC_NO,   KC_NO,      KC_NO, KC_NO, KC_NO,
        KC_ESC,      KC_PSCR, KC_HOME, KC_END,  KC_LEFT,       KC_UP,         /**/ KC_NO,      KC_NO,   KC_NO,      KC_NO, KC_NO, KC_NO,
        KC_NO,       KC_MPLY, KC_MPRV, KC_MNXT, LCTL(KC_LEFT), KC_DOWN,       /**/ KC_NO,      KC_NO,   KC_NO,      KC_NO, KC_NO, KC_NO,
                                       MO(L_LAG), KC_TRNS,     MO(L_LGUI),    /**/ LSFT(KC_R), KC_LGUI, KC_NO
    ),
    [L_EN_SH] = LAYOUT_split_3x6_3(
        LSFT(KC_TAB), LSFT(KC_F), LSFT(KC_P), LSFT(KC_D), LSFT(KC_L), LSFT(KC_X), /**/ LSFT(KC_ENT),  LSFT(KC_U), LSFT(KC_O),    LSFT(KC_Y), LSFT(KC_B),    LSFT(KC_Z),
        LSFT(KC_ESC), LSFT(KC_S), LSFT(KC_N), LSFT(KC_T), LSFT(KC_H), LSFT(KC_K), /**/ LSFT(KC_BSPC), LSFT(KC_A), LSFT(KC_E),    LSFT(KC_I), LSFT(KC_C),    LSFT(KC_Q),
        KC_NO,        LSFT(KC_V), LSFT(KC_W), LSFT(KC_G), LSFT(KC_M), LSFT(KC_J), /**/ KC_NO,         KC_NO,      LSFT(KC_QUOT), KC_NO,      LSFT(KC_SCLN), LSFT(KC_SLSH),
                                              KC_LALT,    KC_LGUI,    KC_LCTL,    /**/ LSFT(KC_R),    KC_TRNS,    MO(L_SWITCH)
    ),
    [L_QW] = LAYOUT_split_3x6_3(
        LSFT_T(KC_TAB), KC_Q, KC_W, KC_E,    KC_R,            KC_T,    /**/ KC_Y,    KC_U,           KC_I,    KC_O,   KC_P,    KC_BSPC,
        KC_ESC,         KC_A, KC_S, KC_D,    KC_F,            KC_G,    /**/ KC_H,    KC_J,           KC_K,    KC_L,   KC_SCLN, KC_QUOT,
        KC_RBRC,        KC_Z, KC_X, KC_C,    KC_V,            KC_B,    /**/ KC_N,    KC_M,           KC_COMM, KC_DOT, KC_SLSH, RCTL_T(KC_ENT),
                                    KC_LALT, LSFT_T(KC_LGUI), KC_LCTL, /**/ KC_LBRC, RSFT_T(KC_SPC), OSL(L_SYMBOL)
    ),
    [L_UA] = LAYOUT_split_3x6_3(
        KC_TAB,  KC_LBRC, KC_U, KC_V,      KC_W,                  KC_I,      /**/ RCTL_T(KC_ENT), KC_E,               KC_J,   KC_B, KC_Q,    KC_SCLN,
        KC_ESC,  KC_K,    KC_Y, KC_R,      KC_N,                  KC_L,      /**/ LCTL(KC_BSPC),  KC_F,               KC_T,   KC_S, KC_C,    KC_M,
        KC_LSFT, KC_X,    KC_G, KC_D,      KC_P,                  KC_COMM,   /**/ KC_QUES,        RSFT_T(KC_SLSH),    KC_GRV, KC_Z, KC_RBRC, KC_A,
                                MO(L_ALT), TD(MO_UASPECIAL_LGUI), MO(L_CTL), /**/ KC_H,           LT(L_UA_SH,KC_SPC), OSL(L_SYMBOL)
    ),
    [L_UA_SPECIAL] = LAYOUT_split_3x6_3(
        LALT(KC_F4), KC_MUTE, KC_VOLD,  KC_VOLU,   KC_RGHT,       LCTL(KC_RGHT), /**/ KC_NO,      LSFT(KC_QUOT), LSFT(KC_BSLS), LSFT(KC_O), LSFT(KC_DOT), KC_NO,
        KC_ESC,      KC_PSCR, KC_HOME,  KC_END,    KC_LEFT,       KC_UP,         /**/ KC_NO,      KC_QUOT,       KC_BSLS,       KC_O,       KC_DOT,       KC_AMPR,
        KC_NO,       KC_MPLY, KC_MPRV,  KC_MNXT,   LCTL(KC_LEFT), KC_DOWN,       /**/ KC_NO,      KC_CIRC,       KC_NO,         KC_EQL,     KC_DLR,       RALT(KC_SLSH),
                                        MO(L_LAG), KC_TRNS,       MO(L_LGUI),    /**/ LSFT(KC_H), KC_LGUI,       KC_NO
    ),
    [L_UA_SH] = LAYOUT_split_3x6_3(
        LSFT(KC_TAB), LSFT(KC_LBRC), LSFT(KC_U), LSFT(KC_V), LSFT(KC_W), LSFT(KC_I),    /**/ LSFT(KC_ENT),  LSFT(KC_E), LSFT(KC_J), LSFT(KC_B), LSFT(KC_Q),    LSFT(KC_SCLN),
        LSFT(KC_ESC), LSFT(KC_K),    LSFT(KC_Y), LSFT(KC_R), LSFT(KC_N), LSFT(KC_L),    /**/ LSFT(KC_BSPC), LSFT(KC_F), LSFT(KC_T), LSFT(KC_S), LSFT(KC_C),    LSFT(KC_M),
        KC_NO,        LSFT(KC_X),    LSFT(KC_G), LSFT(KC_D), LSFT(KC_P), LSFT(KC_COMM), /**/ KC_NO,         KC_NO,      LSFT(KC_2), LSFT(KC_Z), LSFT(KC_RBRC), LSFT(KC_A),
                                                  KC_LALT,   KC_LGUI,    KC_LCTL,       /**/ LSFT(KC_H),    KC_TRNS,    MO(L_SWITCH)
    ),
    [L_CTL] = LAYOUT_split_3x6_3(
        LCTL(KC_TAB), LCTL(KC_F), LCTL(KC_P), LCTL(KC_D), LCTL(KC_L), LCTL(KC_X), /**/ LCTL(KC_ENT),  LCTL(KC_U),   LCTL(KC_O),    LCTL(KC_Y),   LCTL(KC_B),    LCTL(KC_Z),
        LCTL(KC_ESC), LCTL(KC_S), LCTL(KC_N), LCTL(KC_T), LCTL(KC_H), LCTL(KC_K), /**/ LCTL(KC_BSPC), LCTL(KC_A),   LCTL(KC_E),    LCTL(KC_I),   LCTL(KC_C),    LCTL(KC_Q),
        KC_NO,        LCTL(KC_V), LCTL(KC_W), LCTL(KC_G), LCTL(KC_M), LCTL(KC_J), /**/ LCTL(KC_COMM), LCTL(KC_DOT), LCTL(KC_QUOT), LCTL(KC_EQL), LCTL(KC_SCLN), LCTL(KC_SLSH),
                                              KC_LALT,    MO(L_LGUI), KC_TRNS,    /**/ LCTL(KC_R),    KC_RSFT,      KC_NO
    ),
    [L_ALT] = LAYOUT_split_3x6_3(
        LALT(KC_TAB), LALT(KC_F), LALT(KC_P), LALT(KC_D), LALT(KC_L), LALT(KC_X), /**/ LALT(KC_ENT),  LALT(KC_U),   LALT(KC_O),    LALT(KC_Y),   LALT(KC_B),    LALT(KC_Z),
        LALT(KC_ESC), LALT(KC_S), LALT(KC_N), LALT(KC_T), LALT(KC_H), LALT(KC_K), /**/ LALT(KC_BSPC), LALT(KC_A),   LALT(KC_E),    LALT(KC_I),   LALT(KC_C),    LALT(KC_Q),
        KC_NO,        LALT(KC_V), LALT(KC_W), LALT(KC_G), LALT(KC_M), LALT(KC_J), /**/ LALT(KC_COMM), LALT(KC_DOT), LALT(KC_QUOT), LALT(KC_EQL), LALT(KC_SCLN), LALT(KC_SLSH),
                                              KC_TRNS,    MO(L_LGUI), KC_LCTL,    /**/ LALT(KC_R),    KC_RSFT,      KC_NO
    ),
    [L_LGUI] = LAYOUT_split_3x6_3(
        LGUI(KC_TAB), LGUI(KC_F), LGUI(KC_P), LGUI(KC_D), LGUI(KC_L), LGUI(KC_X), /**/ LGUI(KC_ENT),  LGUI(KC_U),   LGUI(KC_O),    LGUI(KC_Y),   LGUI(KC_B),    LGUI(KC_Z),
        LGUI(KC_ESC), LGUI(KC_S), LGUI(KC_N), LGUI(KC_T), LGUI(KC_H), LGUI(KC_K), /**/ LGUI(KC_BSPC), LGUI(KC_A),   LGUI(KC_E),    LGUI(KC_I),   LGUI(KC_C),    LGUI(KC_Q),
        KC_NO,        LGUI(KC_V), LGUI(KC_W), LGUI(KC_G), LGUI(KC_M), LGUI(KC_J), /**/ LGUI(KC_COMM), LGUI(KC_DOT), LGUI(KC_QUOT), LGUI(KC_EQL), LGUI(KC_SCLN), LGUI(KC_SLSH),
                                              KC_NO,      KC_TRNS,    KC_TRNS,    /**/ LGUI(KC_R),    KC_RSFT,      KC_RCTL
    ),
    [L_LAG] = LAYOUT_split_3x6_3(
        LAG(KC_TAB), LAG(KC_F), LAG(KC_P), LAG(KC_D), LAG(KC_L), LAG(KC_X), /**/  LAG(KC_ENT),  LAG(KC_U),   LAG(KC_O),    LAG(KC_Y),   LAG(KC_B),    LAG(KC_Z),
        LAG(KC_ESC), LAG(KC_S), LAG(KC_N), LAG(KC_T), LAG(KC_H), LAG(KC_K), /**/  LAG(KC_BSPC), LAG(KC_A),   LAG(KC_E),    LAG(KC_I),   LAG(KC_C),    LAG(KC_Q),
        KC_NO,       LAG(KC_V), LAG(KC_W), LAG(KC_G), LAG(KC_M), LAG(KC_J), /**/  LAG(KC_COMM), LAG(KC_DOT), LAG(KC_QUOT), LAG(KC_EQL), LAG(KC_SCLN), LAG(KC_SLSH),
                                           KC_TRNS,   KC_TRNS,    KC_NO,    /**/  LAG(KC_R),    KC_RSFT,     KC_RCTL
    ),
    [L_MOUSE] = LAYOUT_split_3x6_3(
        KC_NO, KC_NO, KC_NO, MS_WHLU, MS_RGHT, MS_BTN2, /**/ KC_NO,   KC_NO,  KC_NO,      KC_NO, KC_NO, KC_NO,
        KC_NO, KC_NO, KC_NO, MS_BTN3, MS_LEFT, MS_UP,   /**/ KC_NO,   KC_NO,  KC_NO,      KC_NO, KC_NO, KC_NO,
        KC_NO, KC_NO, KC_NO, MS_WHLD, MS_BTN1, MS_DOWN, /**/ KC_NO,   KC_NO,  KC_NO,      KC_NO, KC_NO, KC_NO,
                             KC_NO,   KC_NO,   KC_NO,   /**/ KC_TRNS, KC_SPC, TG(L_MOUSE)
    ),
    [L_SYMBOL] = LAYOUT_split_3x6_3(
        KC_GRV,  KC_ASTR, KC_UNDS, KC_LPRN, KC_RPRN, KC_BSLS,      /**/ KC_AMPR,     KC_7,         KC_8,    KC_9,    KC_HASH, KC_EXLM,
        KC_CIRC, KC_PLUS, KC_MINS, KC_LCBR, KC_RCBR, LCTL(KC_DEL), /**/ KC_BSPC,     KC_1,         KC_2,    KC_3,    KC_0,    KC_PIPE,
        KC_TILD, KC_LT,   KC_GT,   KC_LBRC, KC_RBRC, KC_DEL,       /**/ KC_DLR,      KC_4,         KC_5,    KC_6,    KC_PERC, KC_AT,
                                   KC_LALT, KC_LGUI, KC_LCTL,      /**/ TO(L_MOUSE), MO(L_SWITCH), KC_TRNS
    ),
    [L_SWITCH] = LAYOUT_split_3x6_3(
        QK_BOOT, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, /**/ DF_QW_LGUISP, KC_NO,        KC_NO,         KC_NO,        KC_NO, KC_NO,
        KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, /**/ KC_NO,        DF_EN_LGUISP, DF_UA_LGUISP,  LGUI(KC_SPC), KC_NO, KC_NO,
        KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, /**/ KC_NO,        KC_NO,        KC_NO,  KC_NO, KC_NO,        KC_NO,
                               KC_NO, KC_NO, KC_NO, /**/ KC_NO,        KC_TRNS,      KC_TRNS
    )
};


#ifdef OTHER_KEYMAP_C
#    include OTHER_KEYMAP_C
#endif // OTHER_KEYMAP_C
