#include <stdint.h>
#include QMK_KEYBOARD_H

#define L_EN 0
#define L_EN_SPECIAL 1
#define L_EN_SH 2
#define L_QW 3
#define L_UA 4
#define L_UA_SPECIAL 5
#define L_UA_SH 6
#define L_GAME 7
#define L_GAME1 8
#define L_CTL 9
#define L_ALT 10
#define L_LGUI 11
#define L_LAG 12
#define L_MOUSE 13
#define L_SYMBOL 14
#define L_SWITCH 15

#define HOT_EN 1
#define HOT_QW 2*2*2
#define HOT_UA 2*2*2*2

bool modifier_active = false;
uint16_t ua_to_en_mapping[] = {
    [KC_SCLN] = KC_F,
    [KC_G] = KC_P,
    [KC_L] = KC_D,
    [KC_X] = KC_L,
    [KC_W] = KC_X,
    [KC_Z] = KC_U,
    [KC_J] = KC_O,
    [KC_E] = KC_Y,
    [KC_A] = KC_B,
    [KC_I] = KC_Z,
    [KC_C] = KC_S,
    [KC_Y] = KC_N,
    [KC_N] = KC_T,
    [KC_R] = KC_H,
    [KC_D] = KC_K,
    [KC_F] = KC_A,
    [KC_T] = KC_E,
    [KC_S] = KC_I,
    [KC_B] = KC_C,
    [KC_LBRC] = KC_Q,
    [KC_P] = KC_V,
    [KC_U] = KC_W,
    [KC_K] = KC_G,
    [KC_V] = KC_M,
    [KC_COMM] = KC_J,
    [KC_QUES] = KC_COMM,
    [KC_SLSH] = KC_DOT,
    [KC_Q] = KC_QUOT,
    [KC_M] = KC_EQL,
    [KC_RBRC] = KC_SCLN,
    [KC_DOT] = KC_SLSH,
    [KC_H] = KC_R,
};
bool is_key_mapped(uint16_t keycode) {
    return keycode < ARRAY_SIZE(ua_to_en_mapping) && 
           ua_to_en_mapping[keycode] != 0;
}
#define MAX_PRESSED_MAPPED_KEYS 16
uint16_t pressed_mapped_keys[MAX_PRESSED_MAPPED_KEYS];
uint8_t num_pressed_mapped_keys = 0;

enum custom_keycodes {
    DFEN_LGUISP = SAFE_RANGE,
    DFQW_LGUISP,
    DFUA_LGUISP
};
void switch_en(void) {
    SEND_STRING(SS_LALT(SS_LCTL("1")));
}
void switch_ua(void) {
    SEND_STRING(SS_LALT(SS_LCTL("2")));
}
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case DFEN_LGUISP:
            if (record->event.pressed) {
                switch_en();
                // if (default_layer_state != HOT_EN && default_layer_state != HOT_QW) {
                //     register_code(KC_LEFT_GUI);
                //     register_code(KC_SPC);
                //     unregister_code(KC_LEFT_GUI);
                //     unregister_code(KC_SPC);
                // }
                set_single_default_layer(L_EN);
            } else {
                unregister_code(KC_LEFT_GUI);
                unregister_code(KC_SPC);
            }
            return false;
        case DFQW_LGUISP:
            if (record->event.pressed) {
                set_single_default_layer(L_QW);
            }
            return false;
        case DFUA_LGUISP:
            if (record->event.pressed) {
                switch_ua();
                // if (default_layer_state != HOT_UA && default_layer_state != HOT_QW) {
                //     register_code(KC_LEFT_GUI);
                //     register_code(KC_SPC);
                //     unregister_code(KC_LEFT_GUI);
                //     unregister_code(KC_SPC);
                // }
                set_single_default_layer(L_UA);
            } else {
                unregister_code(KC_LEFT_GUI);
                unregister_code(KC_SPC);
            }
            return false;
        default:
            if (modifier_active && biton32(default_layer_state) == L_UA) {
                uint16_t base_keycode = keycode & 0xFF;
                uint16_t modifiers = keycode & 0xFF00;

                if (is_key_mapped(base_keycode)) {
                    uint16_t mapped = ua_to_en_mapping[base_keycode];
                    uint16_t final_keycode = modifiers | mapped;

                    if (record->event.pressed) {
                        if (num_pressed_mapped_keys >= MAX_PRESSED_MAPPED_KEYS) {
                            break;
                        }
                        register_code16(final_keycode);
                        pressed_mapped_keys[num_pressed_mapped_keys++] = final_keycode;
                    } else {
                        unregister_code16(final_keycode);
                        for (int i = 0; i < num_pressed_mapped_keys; i++) {
                            if (pressed_mapped_keys[i] == final_keycode) {
                                pressed_mapped_keys[i] = 0;
                            }
                        }
                    }
                    return false;
                }
            }
            break;
    }
    return true;
}

layer_state_t layer_state_set_user(layer_state_t state) {
    static uint8_t last_layer = L_EN;
    uint8_t layer = get_highest_layer(state);
    if (default_layer_state == HOT_UA) {
        if (layer == L_SYMBOL) {
            switch_en();
        } else if (last_layer == L_SYMBOL) {
            switch_ua();
        }
    }
    last_layer = layer;
    return state;
}

enum td_keycodes {
    OSLENSH_LGUI,
    OSLUASH_LGUI,
    LCTL_LALT,
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
static td_tap_t osl_enlsh_lgui_tap_state = {
    .is_press_action = true,
    .state = TD_NONE
};
static td_tap_t osl_ualsh_lgui_tap_state = {
    .is_press_action = true,
    .state = TD_NONE
};
static td_tap_t lctl_lalt_tap_state = {
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
void osl_enlsh_lgui_finished(tap_dance_state_t *state, void *user_data) {
    osl_enlsh_lgui_tap_state.state = cur_dance(state);
    switch (osl_enlsh_lgui_tap_state.state) {
        case TD_SINGLE_TAP:
            set_oneshot_layer(L_EN_SH, ONESHOT_START);
            break;
        case TD_SINGLE_HOLD:
            layer_on(L_EN_SH);
            break;
        case TD_DOUBLE_TAP:
            register_code(KC_LGUI);
            break;
        default:
            break;
    }
}
void osl_ualsh_lgui_finished(tap_dance_state_t *state, void *user_data) {
    osl_ualsh_lgui_tap_state.state = cur_dance(state);
    switch (osl_ualsh_lgui_tap_state.state) {
        case TD_SINGLE_TAP:
            set_oneshot_layer(L_UA_SH, ONESHOT_START);
            break;
        case TD_SINGLE_HOLD:
            layer_on(L_UA_SH);
            break;
        case TD_DOUBLE_TAP:
            register_code(KC_LGUI);
            break;
        default:
            break;
    }
}
void lctl_lalt_finished(tap_dance_state_t *state, void *user_data) {
    lctl_lalt_tap_state.state = cur_dance(state);
    modifier_active = true;
    switch (lctl_lalt_tap_state.state) {
        case TD_SINGLE_TAP:
            register_code(KC_LCTL);
            break;
        case TD_SINGLE_HOLD:
            register_code(KC_LCTL);
            break;
        case TD_DOUBLE_TAP:
            register_code(KC_LALT);
            break;
        default:
            break;
    }
}
void osl_enlsh_lgui_reset(tap_dance_state_t *state, void *user_data) {
    switch (osl_enlsh_lgui_tap_state.state) {
        case TD_SINGLE_TAP:
            clear_oneshot_layer_state(ONESHOT_PRESSED);
            break;
        case TD_SINGLE_HOLD:
            layer_off(L_EN_SH);
            break;
        case TD_DOUBLE_TAP:
            unregister_code(KC_LGUI);
            break;
        default:
            break;
    }
    osl_enlsh_lgui_tap_state.state = TD_NONE;
}
void osl_ualsh_lgui_reset(tap_dance_state_t *state, void *user_data) {
    switch (osl_ualsh_lgui_tap_state.state) {
        case TD_SINGLE_TAP:
            clear_oneshot_layer_state(ONESHOT_PRESSED);
            break;
        case TD_SINGLE_HOLD:
            layer_off(L_UA_SH);
            break;
        case TD_DOUBLE_TAP:
            unregister_code(KC_LGUI);
            break;
        default:
            break;
    }
    osl_enlsh_lgui_tap_state.state = TD_NONE;
}
void lctl_lalt_reset(tap_dance_state_t *state, void *user_data) {
    modifier_active = false;
    for (int i = 0; i < num_pressed_mapped_keys; i++) {
        unregister_code16(pressed_mapped_keys[i]);
    }
    num_pressed_mapped_keys = 0;
    switch (lctl_lalt_tap_state.state) {
        case TD_SINGLE_TAP:
            unregister_code(KC_LCTL);
            break;
        case TD_SINGLE_HOLD:
            unregister_code(KC_LCTL);
            break;
        case TD_DOUBLE_TAP:
            unregister_code(KC_LALT);
            break;
        default:
            break;
    }
    osl_enlsh_lgui_tap_state.state = TD_NONE;
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
    [OSLENSH_LGUI] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, osl_enlsh_lgui_finished, osl_enlsh_lgui_reset),
    [OSLUASH_LGUI] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, osl_ualsh_lgui_finished, osl_ualsh_lgui_reset),
    [LCTL_LALT] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, lctl_lalt_finished, lctl_lalt_reset),
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [L_EN] = LAYOUT_split_3x6_3(
        KC_TAB,  KC_F, KC_P, KC_D,    KC_L,                  KC_X,     /**/  RCTL_T(KC_ENT),  KC_U,                KC_O,    KC_Y,   KC_B,    KC_Z,
        KC_ESC,  KC_S, KC_N, KC_T,    KC_H,                  KC_K,     /**/  LCTL(KC_BSPC),   KC_A,                KC_E,    KC_I,   KC_C,    KC_Q,
        KC_LSFT, KC_V, KC_W, KC_G,    KC_M,                  KC_J,     /**/  RALT_T(KC_COMM), RSFT_T(KC_DOT),      KC_QUOT, KC_EQL, KC_SCLN, KC_SLSH,
                             OSL(L_EN_SPECIAL), TD(OSLENSH_LGUI), TD(LCTL_LALT),  /**/  KC_R,            KC_SPC, OSL(L_SYMBOL)
    ),
    [L_EN_SPECIAL] = LAYOUT_split_3x6_3(
        LALT(KC_F4), KC_MUTE,    KC_VOLD, KC_VOLU, KC_RGHT,       LCTL(KC_RGHT), /**/ KC_NO, KC_NO,   KC_NO,      KC_NO, KC_NO, KC_PSCR,
        LCTL(KC_A),  LCTL(KC_C), KC_HOME, KC_END,  KC_LEFT,       KC_UP,         /**/ KC_NO, KC_NO,   KC_NO,      KC_NO, KC_NO, KC_NO,
        LCTL(KC_Z),  KC_MPLY,    KC_MPRV, KC_MNXT, LCTL(KC_LEFT), KC_DOWN,       /**/ KC_NO, KC_NO,   KC_NO,      KC_NO, KC_NO, KC_NO,
                                       KC_TRNS, KC_LGUI,       TD(LCTL_LALT), /**/ KC_NO, KC_SPC,  KC_NO
    ),
    [L_EN_SH] = LAYOUT_split_3x6_3(
        LSFT(KC_TAB), LSFT(KC_F), LSFT(KC_P), LSFT(KC_D), LSFT(KC_L), LSFT(KC_X), /**/ LSFT(KC_ENT),  LSFT(KC_U), LSFT(KC_O),    LSFT(KC_Y), LSFT(KC_B),    LSFT(KC_Z),
        LSFT(KC_ESC), LSFT(KC_S), LSFT(KC_N), LSFT(KC_T), LSFT(KC_H), LSFT(KC_K), /**/ LSFT(KC_BSPC), LSFT(KC_A), LSFT(KC_E),    LSFT(KC_I), LSFT(KC_C),    LSFT(KC_Q),
        KC_NO,        LSFT(KC_V), LSFT(KC_W), LSFT(KC_G), LSFT(KC_M), LSFT(KC_J), /**/ KC_NO,         KC_NO,      LSFT(KC_QUOT), KC_NO,      LSFT(KC_SCLN), LSFT(KC_SLSH),
                                              MO(L_LAG),  KC_TRNS,    MO(L_LGUI), /**/ LSFT(KC_R),    LSFT(KC_SPC), KC_NO
    ),
    [L_QW] = LAYOUT_split_3x6_3(
        LSFT_T(KC_TAB), KC_Q, KC_W, KC_E,    KC_R,            KC_T,  /**/ KC_Y,    KC_U,   KC_I,    KC_O,   KC_P,    KC_BSPC,
        KC_ESC,         KC_A, KC_S, KC_D,    KC_F,            KC_G,  /**/ KC_H,    KC_J,   KC_K,    KC_L,   KC_SCLN, KC_QUOT,
        KC_RBRC,        KC_Z, KC_X, KC_C,    KC_V,            KC_B,  /**/ KC_N,    KC_M,   KC_COMM, KC_DOT, KC_SLSH, RCTL_T(KC_ENT),
                                    KC_LGUI, KC_RSFT, TD(LCTL_LALT), /**/ KC_LBRC, KC_SPC, OSL(L_SYMBOL)
    ),
    [L_GAME] = LAYOUT_split_3x6_3(
        KC_ESC,  KC_1, KC_Q, KC_W,   KC_E,    KC_R,        /**/ KC_T,    KC_Y,           KC_U,           KC_I,    KC_O,    KC_P,
        KC_LALT, KC_2, KC_A, KC_S,   KC_D,    KC_F,        /**/ KC_G,    KC_H,           KC_J,           KC_K,    KC_UP,   KC_L,
        KC_LCTL, KC_3, KC_T, KC_G,   KC_B,    KC_C,        /**/ KC_B,    KC_N,           KC_LCTL,        KC_LEFT, KC_DOWN, KC_RGHT,
                             KC_SPC, KC_LSFT, MO(L_GAME1), /**/ KC_LBRC, KC_SPC, OSL(L_SYMBOL)
    ),
    [L_GAME1] = LAYOUT_split_3x6_3(
        KC_ESC, KC_4, KC_F3, KC_W,   KC_F5,   KC_Y,    /**/ KC_T,    KC_Y,           KC_U,           KC_I,    KC_O,   KC_P,
        KC_TAB, KC_5, KC_A,  KC_S,   KC_D,    KC_H,    /**/ KC_G,    KC_H,           KC_J,           KC_K,    KC_L,   KC_SCLN,
        KC_LSFT,KC_6, KC_Z,  KC_X,   KC_V,    KC_N,    /**/ KC_B,    KC_N,           KC_M,           KC_COMM, KC_DOT, KC_SLSH,
                             KC_SPC, KC_LSFT, KC_TRNS, /**/ KC_LBRC, KC_SPC, OSL(L_SYMBOL)
    ),
    [L_UA] = LAYOUT_split_3x6_3(
        KC_TAB,  KC_SCLN, KC_G, KC_L,      KC_X,                     KC_W,          /**/ RCTL_T(KC_ENT), KC_Z,            KC_J, KC_E, KC_A,    KC_I,
        KC_ESC,  KC_C,    KC_Y, KC_N,      KC_R,                     KC_D,          /**/ LCTL(KC_BSPC),  KC_F,            KC_T, KC_S, KC_B,    KC_LBRC,
        KC_LSFT, KC_P,    KC_U, KC_K,      KC_V,                     KC_COMM,       /**/ KC_QUES,        RSFT_T(KC_SLSH), KC_Q, KC_M, KC_RBRC, KC_DOT,
                                OSL(L_UA_SPECIAL), TD(OSLUASH_LGUI), TD(LCTL_LALT), /**/ KC_H,   KC_SPC,          OSL(L_SYMBOL)
    ),
    [L_UA_SPECIAL] = LAYOUT_split_3x6_3(
        LALT(KC_F4), KC_MUTE,    KC_VOLD,  KC_VOLU, KC_RGHT,       LCTL(KC_RGHT), /**/ KC_NO, LSFT(KC_QUOT), LSFT(KC_BSLS), LSFT(KC_O), KC_NO,    KC_PSCR,
        LCTL(KC_A),  LCTL(KC_C), KC_HOME,  KC_END,  KC_LEFT,       KC_UP,         /**/ KC_NO, KC_QUOT,       KC_BSLS,       KC_O,       KC_CIRC,  KC_AMPR,
        LCTL(KC_Z),  KC_MPLY,    KC_MPRV,  KC_MNXT, LCTL(KC_LEFT), KC_DOWN,       /**/ KC_NO, LSFT(KC_2),    KC_GRV,        KC_EQL,     KC_DLR,   RALT(KC_SLSH),
                                        KC_TRNS, KC_LGUI,       TD(LCTL_LALT), /**/ KC_NO, KC_SPC,        KC_NO
    ),
    [L_UA_SH] = LAYOUT_split_3x6_3(
        LSFT(KC_TAB), LSFT(KC_SCLN), LSFT(KC_G), LSFT(KC_L), LSFT(KC_X), LSFT(KC_W),    /**/ LSFT(KC_ENT),  LSFT(KC_Z),   LSFT(KC_J), LSFT(KC_E), LSFT(KC_A),    LSFT(KC_I),
        LSFT(KC_ESC), LSFT(KC_C),    LSFT(KC_Y), LSFT(KC_N), LSFT(KC_R), LSFT(KC_D),    /**/ LSFT(KC_BSPC), LSFT(KC_F),   LSFT(KC_T), LSFT(KC_S), LSFT(KC_B),    LSFT(KC_LBRC),
        KC_NO,        LSFT(KC_P),    LSFT(KC_U), LSFT(KC_K), LSFT(KC_V), LSFT(KC_COMM), /**/ KC_NO,         KC_NO,        LSFT(KC_Q), LSFT(KC_M), LSFT(KC_RBRC), LSFT(KC_DOT),
                                                 MO(L_LAG),  KC_TRNS,    MO(L_LGUI),    /**/ LSFT(KC_H),    LSFT(KC_SPC), KC_NO
    ),
    [L_LGUI] = LAYOUT_split_3x6_3(
        LGUI(KC_TAB), LGUI(KC_F), LGUI(KC_P), LGUI(KC_D), LGUI(KC_L), LGUI(KC_X), /**/ LGUI(KC_ENT),  LGUI(KC_U),   LGUI(KC_O),    LGUI(KC_Y),   LGUI(KC_B),    LGUI(KC_Z),
        LGUI(KC_ESC), LGUI(KC_S), LGUI(KC_N), LGUI(KC_T), LGUI(KC_H), LGUI(KC_K), /**/ LGUI(KC_BSPC), LGUI(KC_A),   LGUI(KC_E),    LGUI(KC_I),   LGUI(KC_C),    LGUI(KC_Q),
        KC_NO,        LGUI(KC_V), LGUI(KC_W), LGUI(KC_G), LGUI(KC_M), LGUI(KC_J), /**/ LGUI(KC_COMM), LGUI(KC_DOT), LGUI(KC_QUOT), LGUI(KC_EQL), LGUI(KC_SCLN), LGUI(KC_SLSH),
                                              KC_NO,      KC_TRNS,    KC_TRNS,    /**/ LGUI(KC_R),    LGUI(KC_LSFT), KC_RCTL
    ),
    [L_LAG] = LAYOUT_split_3x6_3(
        LAG(KC_TAB), LAG(KC_F), LAG(KC_P), LAG(KC_D), LAG(KC_L), LAG(KC_X), /**/  LAG(KC_ENT),  LAG(KC_U),   LAG(KC_O),    LAG(KC_Y),   LAG(KC_B),    LAG(KC_Z),
        LAG(KC_ESC), LAG(KC_S), LAG(KC_N), LAG(KC_T), LAG(KC_H), LAG(KC_K), /**/  LAG(KC_BSPC), LAG(KC_A),   LAG(KC_E),    LAG(KC_I),   LAG(KC_C),    LAG(KC_Q),
        KC_NO,       LAG(KC_V), LAG(KC_W), LAG(KC_G), LAG(KC_M), LAG(KC_J), /**/  LAG(KC_COMM), LAG(KC_DOT), LAG(KC_QUOT), LAG(KC_EQL), LAG(KC_SCLN), LAG(KC_SLSH),
                                           KC_TRNS,   KC_TRNS,    KC_NO,    /**/  LAG(KC_R),    LAG(KC_LSFT), KC_RCTL
    ),
    [L_MOUSE] = LAYOUT_split_3x6_3(
        KC_NO, KC_NO, KC_NO, MS_WHLU, MS_RGHT, MS_BTN2, /**/ KC_NO,   KC_NO,  KC_NO,      KC_NO, KC_NO, KC_NO,
        KC_NO, KC_NO, KC_NO, MS_BTN3, MS_LEFT, MS_UP,   /**/ KC_NO,   KC_NO,  KC_NO,      KC_NO, KC_NO, KC_NO,
        KC_NO, KC_NO, KC_NO, MS_WHLD, MS_BTN1, MS_DOWN, /**/ KC_NO,   KC_NO,  KC_NO,      KC_NO, KC_NO, KC_NO,
                             KC_LGUI, KC_LSFT, TD(LCTL_LALT), /**/ KC_TRNS, KC_SPC, TG(L_MOUSE)
    ),
    [L_SYMBOL] = LAYOUT_split_3x6_3(
        KC_GRV,  KC_ASTR, KC_UNDS, KC_LPRN, KC_RPRN, KC_BSLS,      /**/ KC_AMPR,     KC_7,         KC_8,    KC_9,    KC_HASH, KC_EXLM,
        KC_CIRC, KC_PLUS, KC_MINS, KC_LCBR, KC_RCBR, LCTL(KC_DEL), /**/ KC_BSPC,     KC_1,         KC_2,    KC_3,    KC_0,    KC_PIPE,
        KC_TILD, KC_LT,   KC_GT,   KC_LBRC, KC_RBRC, KC_DEL,       /**/ KC_DLR,      KC_4,         KC_5,    KC_6,    KC_PERC, KC_AT,
                                   KC_SPC,  KC_LSFT, TD(LCTL_LALT),/**/ TO(L_MOUSE), MO(L_SWITCH), KC_TRNS
    ),
    [L_SWITCH] = LAYOUT_split_3x6_3(
        QK_BOOT, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, /**/ DFQW_LGUISP, KC_NO,        KC_NO,         KC_NO,        KC_NO, KC_NO,
        KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, /**/ DF(L_GAME),  DFEN_LGUISP, DFUA_LGUISP,  LGUI(KC_SPC), KC_NO, KC_NO,
        KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, /**/ KC_NO,        KC_NO,        KC_NO,  KC_NO, KC_NO,        KC_NO,
                               KC_NO, KC_NO, KC_NO, /**/ KC_NO,        KC_TRNS,      KC_TRNS
    )
};


#ifdef OTHER_KEYMAP_C
#    include OTHER_KEYMAP_C
#endif // OTHER_KEYMAP_C
