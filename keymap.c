#include <stdint.h>
#include QMK_KEYBOARD_H
#include "os_detection.h"

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

enum custom_keycodes {
    DFEN_LGUISP = SAFE_RANGE,
    DFQW_LGUISP,
    DFUA_LGUISP,
};

static bool is_mac_host = false;
static bool modifier_active = false;

static void apply_host_os(void) {
    keymap_config.swap_lctl_lgui = is_mac_host;
    keymap_config.swap_rctl_rgui = is_mac_host;
}

bool process_detected_host_os_user(os_variant_t os) {
    is_mac_host = (os == OS_MACOS || os == OS_IOS);
    apply_host_os();
    return true;
}

static uint16_t shortcut_mod(void) {
    return is_mac_host ? KC_LGUI : KC_LCTL;
}

static uint16_t secondary_mod(void) {
    return is_mac_host ? KC_LCTL : KC_LGUI;
}

typedef struct {
    uint16_t from;
    uint8_t  mods;
    uint16_t to;
} mac_override_t;

static const mac_override_t mac_overrides[] = {
    {KC_HOME,       MOD_LGUI,            KC_LEFT},
    {KC_END,        MOD_LGUI,            KC_RGHT},
    {LCTL(KC_LEFT), MOD_LALT,            KC_LEFT},
    {LCTL(KC_RGHT), MOD_LALT,            KC_RGHT},
    {LCTL(KC_BSPC), MOD_LALT,            KC_BSPC},
    {LCTL(KC_DEL),  MOD_LALT,            KC_DEL},
    {KC_PSCR,       MOD_LGUI | MOD_LSFT, KC_4},
    {LALT(KC_F4),   MOD_LGUI,            KC_W},
};

static void hold_release(uint8_t mods, uint16_t keycode, bool pressed) {
    if (pressed) {
        if (mods) register_mods(mods);
        if (keycode != KC_NO) register_code(keycode);
    } else {
        if (keycode != KC_NO) unregister_code(keycode);
        if (mods) unregister_mods(mods);
    }
}

static bool process_mac_remap(uint16_t keycode, keyrecord_t *record) {
    if (!is_mac_host) return true;

    for (uint8_t i = 0; i < ARRAY_SIZE(mac_overrides); i++) {
        if (mac_overrides[i].from == keycode) {
            hold_release(mac_overrides[i].mods, mac_overrides[i].to,
                         record->event.pressed);
            return false;
        }
    }

    if (IS_QK_MODS(keycode) &&
        QK_MODS_GET_MODS(keycode) == (MOD_LALT | MOD_LGUI)) {
        hold_release(MOD_LGUI | MOD_LCTL, QK_MODS_GET_BASIC_KEYCODE(keycode),
                     record->event.pressed);
        return false;
    }
    return true;
}

static const uint16_t ua_to_en_mapping[] = {
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
    [KC_SLSH] = KC_DOT,
    [KC_Q] = KC_QUOT,
    [KC_M] = KC_EQL,
    [KC_RBRC] = KC_SCLN,
    [KC_DOT] = KC_SLSH,
    [KC_H] = KC_R,
};

static bool is_key_mapped(uint8_t keycode) {
    return keycode < ARRAY_SIZE(ua_to_en_mapping) &&
           ua_to_en_mapping[keycode] != 0;
}

#define MAX_PRESSED_MAPPED_KEYS 8

typedef struct {
    keypos_t key;
    uint16_t keycode;
} mapped_key_t;

static mapped_key_t pressed_mapped_keys[MAX_PRESSED_MAPPED_KEYS];
static uint8_t num_pressed_mapped_keys = 0;

static int8_t find_mapped_key(keypos_t key) {
    for (uint8_t i = 0; i < num_pressed_mapped_keys; i++) {
        if (pressed_mapped_keys[i].key.row == key.row &&
            pressed_mapped_keys[i].key.col == key.col) {
            return (int8_t)i;
        }
    }
    return -1;
}

// Releases are resolved from the table alone, never re-derived from the
// current keycode or modifier state, so a key mapped on press is always
// unmapped on release even if the layer or modifier changed in between.
static bool process_ua_ctrl_remap(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) {
        int8_t index = find_mapped_key(record->event.key);
        if (index < 0) {
            return true;
        }
        unregister_code16(pressed_mapped_keys[index].keycode);
        pressed_mapped_keys[index] = pressed_mapped_keys[--num_pressed_mapped_keys];
        return false;
    }

    if (is_mac_host || !modifier_active ||
        biton32(default_layer_state) != L_UA ||
        num_pressed_mapped_keys >= MAX_PRESSED_MAPPED_KEYS ||
        keycode > QK_MODS_MAX) {
        return true;
    }

    uint8_t base = keycode & 0xFF;
    if (!is_key_mapped(base)) {
        return true;
    }

    uint16_t final_keycode = (keycode & 0xFF00) | ua_to_en_mapping[base];
    register_code16(final_keycode);
    pressed_mapped_keys[num_pressed_mapped_keys++] = (mapped_key_t){
        .key     = record->event.key,
        .keycode = final_keycode,
    };
    return false;
}

#define LAYOUT_HOTKEY_MODS (MOD_LCTL | MOD_LALT)
#define LAYOUT_SWITCH_WAIT_MS 40

// Mods are cleared around the chord because SEND_STRING-style output adds to
// the live modifier state rather than replacing it; a held shifted symbol such
// as KC_DLR would otherwise corrupt it. Weak mods cover ACT_MODS keymap
// entries, real mods cover a physically held KC_LSFT.
//
// register_mods bypasses mod_config, so this emits real Ctrl on macOS too,
// which is what the host-side hotkey handler binds.
//
// The trailing wait stalls the scan loop until the host has plausibly applied
// the new layout. There is no acknowledgment path, so without it a key pressed
// immediately after the layer change is emitted under the old layout.
static void send_layout_hotkey(uint16_t keycode) {
    uint8_t saved_mods = get_mods();
    uint8_t saved_weak_mods = get_weak_mods();

    clear_mods();
    clear_weak_mods();
    send_keyboard_report();

    register_mods(LAYOUT_HOTKEY_MODS);
    tap_code(keycode);
    unregister_mods(LAYOUT_HOTKEY_MODS);

    set_mods(saved_mods);
    set_weak_mods(saved_weak_mods);
    send_keyboard_report();

    wait_ms(LAYOUT_SWITCH_WAIT_MS);
}

static void switch_en(void) {
    send_layout_hotkey(KC_1);
}

static void switch_ua(void) {
    send_layout_hotkey(KC_2);
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!process_mac_remap(keycode, record)) return false;

    switch (keycode) {
        case OS_TOGG:
            if (record->event.pressed) {
                is_mac_host = !is_mac_host;
                apply_host_os();
            }
            return false;
        case DFEN_LGUISP:
            if (record->event.pressed) {
                switch_en();
                set_single_default_layer(L_EN);
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
                set_single_default_layer(L_UA);
            }
            return false;
        default:
            return process_ua_ctrl_remap(keycode, record);
    }
}

layer_state_t layer_state_set_user(layer_state_t state) {
    static bool was_symbol_active = false;
    bool is_symbol_active = (state & (1UL << L_SYMBOL)) != 0;

    if (is_symbol_active != was_symbol_active) {
        was_symbol_active = is_symbol_active;
        if (biton32(default_layer_state) == L_UA) {
            if (is_symbol_active) {
                switch_en();
            } else {
                switch_ua();
            }
        }
    }
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

static td_state_t cur_dance(tap_dance_state_t *state) {
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
            register_code(secondary_mod());
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
            register_code(secondary_mod());
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
        case TD_SINGLE_HOLD:
            register_code(shortcut_mod());
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
            unregister_code(secondary_mod());
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
            unregister_code(secondary_mod());
            break;
        default:
            break;
    }
    osl_ualsh_lgui_tap_state.state = TD_NONE;
}

void lctl_lalt_reset(tap_dance_state_t *state, void *user_data) {
    modifier_active = false;
    switch (lctl_lalt_tap_state.state) {
        case TD_SINGLE_TAP:
        case TD_SINGLE_HOLD:
            unregister_code(shortcut_mod());
            break;
        case TD_DOUBLE_TAP:
            unregister_code(KC_LALT);
            break;
        default:
            break;
    }
    lctl_lalt_tap_state.state = TD_NONE;
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
        KC_LCTL, KC_3, KC_T, KC_G,   KC_B,    KC_C,        /**/ KC_B,    KC_N,           KC_RCTL,        KC_LEFT, KC_DOWN, KC_RGHT,
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
        OS_TOGG, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, /**/ DF(L_GAME),  DFEN_LGUISP, DFUA_LGUISP,  LGUI(KC_SPC), KC_NO, KC_NO,
        KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, /**/ KC_NO,        KC_NO,        KC_NO,  KC_NO, KC_NO,        KC_NO,
                               KC_NO, KC_NO, KC_NO, /**/ KC_NO,        KC_TRNS,      KC_TRNS
    )
};


#ifdef OTHER_KEYMAP_C
#    include OTHER_KEYMAP_C
#endif // OTHER_KEYMAP_C
