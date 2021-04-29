/* Copyright 2020 ipetepete
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 2 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  */

#include <string.h>
#include "raw_hid.h"
#include "usb_descriptor.h"
#ifdef DEBUG_LAYER
#include "eeconfig.h"
#endif
#include "led_layout.h"
#include QMK_KEYBOARD_H

enum layers {
    _qwerty,
    _spcfn,
    _mouse,
    _fn,
#ifdef DEBUG_LAYER
    _debug,
#endif
};

enum keycodes {
    REC_TOG = SAFE_RANGE,
    REN_TOG,
    WPM_TOG,
    MAC_LOC, // macOS lock screen (LCTL+LGUI+Q)
    ESC_CL, // Escape on tap, Caps Lock on hold
#ifdef DEBUG_LAYER
    DB_TOGG,
    DB_MTRX,
    DB_KBRD,
    DB_MOUS,
    PRT_VER,
    CLR_EEP,
#endif
};

enum td_keycodes {
    TD_M_NXT_PRV,
};

enum output_messages {
    RECORDING_UPDATE,
    WPM_KEYPRESS,
};

enum input_messages {
    RECORDING_QUERY,
    RECORDING_ACK,
};

static uint16_t esc_cl_timer;
static bool esc_cl_pressed = false;

typedef union {
  uint32_t raw;
  struct {
    bool recording_enabled:1;
    bool wpm_enabled:1;
  };
} user_config_t;

user_config_t user_config;

static uint8_t hid_buf[RAW_EPSIZE];

#define RECORDING_RGB_COLOR RGB_RED

static bool local_recording = false;
static bool remote_recording = false;
static uint16_t rec_tog_timer = 0;

static void send_recording(void) {
    memset(hid_buf, 0, sizeof(hid_buf));

    hid_buf[0] = RECORDING_UPDATE;
    hid_buf[1] = local_recording;

    raw_hid_send(hid_buf, sizeof(hid_buf));
}

static void set_recording(bool recording) {
    local_recording = recording;
    send_recording();
}

static void set_recording_enabled(bool recording_enabled) {
    user_config.recording_enabled = recording_enabled;
    eeconfig_update_user(user_config.raw);
    set_recording(remote_recording = !recording_enabled);
}

void keyboard_post_init_user(void) {
    user_config.raw = eeconfig_read_user();
    set_recording(!user_config.recording_enabled);
}

void raw_hid_receive(uint8_t *data, uint8_t length) {
    if (length == 0) return;
    switch (data[0]) {
        case RECORDING_QUERY:
            send_recording();
            break;
        case RECORDING_ACK:
            remote_recording = data[1] != 0;
            break;
    }
}

static uint16_t wpm_timer = 0;
static uint8_t wpm_last = 0;

static void send_wpm(uint8_t wpm, bool is_keypress) {
    wpm_timer = timer_read();
    wpm_last = wpm;

    memset(hid_buf, 0, sizeof(hid_buf));

    hid_buf[0] = WPM_KEYPRESS;
    hid_buf[1] = wpm;
    hid_buf[2] = is_keypress;

    raw_hid_send(hid_buf, sizeof(hid_buf));
}

static void process_wpm(uint8_t wpm, bool is_keypress) {
    if (wpm_last != 0 || wpm != 0 || is_keypress) {
        send_wpm(wpm, is_keypress);
    }
}

static void set_wpm_enabled(bool wpm_enabled) {
    user_config.wpm_enabled = wpm_enabled;
    eeconfig_update_user(user_config.raw);
}

#ifdef DEBUG_LAYER

static void print_version(void) {
    // print version & information
    print("\t- Version -\n");
    print("VID: " STR(VENDOR_ID) "(" STR(MANUFACTURER) ") "
                                                       "PID: " STR(PRODUCT_ID) "(" STR(PRODUCT) ") "
                                                                                                "VER: " STR(DEVICE_VER) "\n");
    print("BUILD:  (" __DATE__ ")\n");
#ifndef SKIP_VERSION
#    ifdef PROTOCOL_CHIBIOS
    print("CHIBIOS: " STR(CHIBIOS_VERSION) ", CONTRIB: " STR(CHIBIOS_CONTRIB_VERSION) "\n");
#    endif
#endif

    /* build options */
    print("OPTIONS:"

#ifdef PROTOCOL_LUFA
          " LUFA"
#endif
#ifdef PROTOCOL_VUSB
          " VUSB"
#endif
#ifdef BOOTMAGIC_ENABLE
          " BOOTMAGIC"
#endif
#ifdef MOUSEKEY_ENABLE
          " MOUSEKEY"
#endif
#ifdef EXTRAKEY_ENABLE
          " EXTRAKEY"
#endif
#ifdef CONSOLE_ENABLE
          " CONSOLE"
#endif
#ifdef COMMAND_ENABLE
          " COMMAND"
#endif
#ifdef NKRO_ENABLE
          " NKRO"
#endif
#ifdef LTO_ENABLE
          " LTO"
#endif

          " " STR(BOOTLOADER_SIZE) "\n");

    print("GCC: " STR(__GNUC__) "." STR(__GNUC_MINOR__) "." STR(__GNUC_PATCHLEVEL__)
#if defined(__AVR__)
              " AVR-LIBC: " __AVR_LIBC_VERSION_STRING__ " AVR_ARCH: avr" STR(__AVR_ARCH__)
#endif
                  "\n");

    return;
}

#define MO_DEBUG MO(_debug)

#else

#define MO_DEBUG XXXXXXX

#endif

#define KC_SPFN LT(_spcfn, KC_SPC) // press for space, hold for function layer (aka spacefn)
#define T_MOUSE TG(_mouse)
#define NXT_PRV TD(TD_M_NXT_PRV)

qk_tap_dance_action_t tap_dance_actions[] = {
    [TD_M_NXT_PRV] = ACTION_TAP_DANCE_DOUBLE(KC_MNXT, KC_MPRV),
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_qwerty] = LAYOUT_60_ansi(
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS,
        ESC_CL,  KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,          KC_ENT,
        KC_LSFT,          KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,          KC_RSFT,
        KC_LCTL, KC_LGUI, KC_LALT,                            KC_SPFN,                            KC_RALT, REC_TOG, KC_RCTL, MO(_fn)
    ),
    [_spcfn] = LAYOUT_60_ansi(
        MAC_LOC, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_DEL,
        _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_PGUP, KC_UP,   KC_PGDN, KC_PSCR, KC_SLCK, KC_PAUS, KC_INS,
        _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_HOME, KC_LEFT, KC_DOWN, KC_RGHT, KC_END,  XXXXXXX,          _______,
        _______,          XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,          _______,
        _______, _______, _______,                            _______,                            _______, _______, _______, _______
    ),
    [_mouse] = LAYOUT_60_ansi(
        XXXXXXX, KC_ACL0, KC_ACL1, KC_ACL2, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, _______,
        _______, XXXXXXX, KC_WH_U, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_BTN1, KC_MS_U, KC_BTN2, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        _______, KC_WH_L, KC_WH_D, KC_WH_R, XXXXXXX, XXXXXXX, XXXXXXX, KC_MS_L, KC_MS_D, KC_MS_R, XXXXXXX, XXXXXXX,          _______,
        _______,          XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,          _______,
        _______, _______, _______,                            XXXXXXX,                            _______, _______, _______, _______
    ),
    [_fn] = LAYOUT_60_ansi(
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, MO_DEBUG,
        XXXXXXX, RGB_TOG, WPM_TOG, RGB_HUI, RGB_HUD, RGB_SAI, RGB_SAD, RGB_VAI, RGB_VAD, RGB_MOD, RGB_RMOD,KC_BRID, KC_BRIU, RESET,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, RGB_SPI, RGB_SPD, XXXXXXX, XXXXXXX,          XXXXXXX,
        KC_MPLY,          KC_VOLD, KC_VOLU, KC_MUTE, XXXXXXX, XXXXXXX, NK_TOGG, T_MOUSE, XXXXXXX, XXXXXXX, XXXXXXX,          NXT_PRV,
        XXXXXXX, XXXXXXX, XXXXXXX,                            KC_SPC,                             XXXXXXX, REN_TOG, XXXXXXX, _______
    ),
#ifdef DEBUG_LAYER
    [_debug] = LAYOUT_60_ansi(
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, _______,
        XXXXXXX, XXXXXXX, XXXXXXX, CLR_EEP, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, DB_TOGG, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, DB_KBRD, XXXXXXX, XXXXXXX, XXXXXXX,          XXXXXXX,
        XXXXXXX,          XXXXXXX, DB_MTRX, XXXXXXX, PRT_VER, XXXXXXX, XXXXXXX, DB_MOUS, XXXXXXX, XXXXXXX, XXXXXXX,          XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX,                            XXXXXXX,                            XXXXXXX, XXXXXXX, XXXXXXX, _______
    ),
#endif
};

#ifndef REC_TOG_TERM
#define REC_TOG_TERM TAPPING_TERM
#endif

#ifndef ESC_CL_TERM
#define ESC_CL_TERM TAPPING_TERM
#endif

#ifndef RGB_TUNING_KEYCODE_REPEAT_INTERVAL
#define RGB_TUNING_KEYCODE_REPEAT_INTERVAL TAPPING_TERM
#endif

static uint16_t rgb_keycode;
static keyrecord_t rgb_record;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case RGB_HUI...RGB_SPD:
            rgb_keycode = keycode;
            rgb_record = *record;
            return true;
        case REC_TOG:
            if (user_config.recording_enabled) {
                if (record->event.pressed) {
                    rec_tog_timer = record->event.time;
                    set_recording(!local_recording);
                } else {
                    if (timer_elapsed(rec_tog_timer) > REC_TOG_TERM) {
                        set_recording(!local_recording);
                    }
                }
            }
            return false;
        case REN_TOG:
            if (record->event.pressed) {
                set_recording_enabled(!user_config.recording_enabled);
            }
            return false;
        case WPM_TOG:
            if (record->event.pressed) {
                set_wpm_enabled(!user_config.wpm_enabled);
            }
            return false;
        case MAC_LOC:
            if (record->event.pressed) {
                tap_code16(LCTL(LGUI(KC_Q)));
            }
            return false;
        case ESC_CL:
            esc_cl_pressed = record->event.pressed;
            if (record->event.pressed) {
                esc_cl_timer = record->event.time;
            } else if (timer_elapsed(esc_cl_timer) < ESC_CL_TERM) {
                tap_code(KC_ESC);
            } else {
                unregister_code(KC_CAPS);
            }
            return false;
#ifdef DEBUG_LAYER
        case DB_TOGG:
            if (record->event.pressed) {
                debug_enable = !debug_enable;
            }
            return false;
        case DB_MTRX:
            if (record->event.pressed) {
                debug_matrix = !debug_matrix;
            }
            return false;
        case DB_KBRD:
            if (record->event.pressed) {
                debug_keyboard = !debug_keyboard;
            }
            return false;
        case DB_MOUS:
            if (record->event.pressed) {
                debug_mouse = !debug_mouse;
            }
            return false;
        case PRT_VER:
            if (record->event.pressed) {
                print_version();
            }
            return false;
        case CLR_EEP:
            if (record->event.pressed) {
                eeconfig_init();
            }
            return false;
#endif
        default:
            return true; // Process all other keycodes normally
    }
}

void post_process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (user_config.wpm_enabled && record->event.pressed && wpm_keycode_user(keycode)) {
        process_wpm(get_current_wpm(), true);
    }
}

#define RGB_DIM_MAGENTA 0x7F, 0x00, 0x7F
#define RGB_DIM_YELLOW 0x7F, 0x7F, 0x00
#define RGB_DIM_CYAN 0x00, 0x7F, 0x7F
#define RGB_DIM_WHITE 0x7F, 0x7F, 0x7F

static void rgb_matrix_set_toggle(int led_index, bool toggle_enabled) {
    if (toggle_enabled) {
        rgb_matrix_set_color(led_index, RGB_GREEN);
    } else {
        rgb_matrix_set_color(led_index, RGB_RED);
    }
}

static void rgb_matrix_layer_helper(uint8_t red, uint8_t green, uint8_t blue, uint8_t led_type) {
    for (int i = 0; i < DRIVER_LED_TOTAL; i++) {
        if (HAS_FLAGS(g_led_config.flags[i], led_type)) {
            rgb_matrix_set_color(i, red, green, blue);
        }
    }
}

void rgb_matrix_indicators_user(void) {
    if (rgb_record.event.pressed && timer_elapsed(rgb_record.event.time) >= RGB_TUNING_KEYCODE_REPEAT_INTERVAL) {
        rgb_record.event.time += RGB_TUNING_KEYCODE_REPEAT_INTERVAL;
        process_rgb(rgb_keycode, &rgb_record);
    }

    if (esc_cl_pressed && timer_elapsed(esc_cl_timer) >= ESC_CL_TERM) {
        esc_cl_pressed = false; // prevent multiple calls to register_code()
        register_code(KC_CAPS);
    }

    if (user_config.wpm_enabled && timer_elapsed(wpm_timer) > 1000) {
        process_wpm(get_current_wpm(), false);
    }

    if (g_suspend_state || !rgb_matrix_config.enable) return;

    if (user_config.recording_enabled) {
        if (local_recording) {
            rgb_matrix_set_color(LL_MENU, RECORDING_RGB_COLOR);
        }
        if (remote_recording) {
            rgb_matrix_layer_helper(RECORDING_RGB_COLOR, LED_FLAG_UNDERGLOW);
        }
    }

    switch (get_highest_layer(layer_state)) {
        case _mouse:
            // Mouse speed
            rgb_matrix_set_color(LL_1, RGB_DIM_YELLOW);
            rgb_matrix_set_color(LL_2, RGB_DIM_YELLOW);
            rgb_matrix_set_color(LL_3, RGB_DIM_YELLOW);
            // Mouse wheel up, left, down and right
            rgb_matrix_set_color(LL_W, RGB_DIM_MAGENTA);
            rgb_matrix_set_color(LL_A, RGB_DIM_MAGENTA);
            rgb_matrix_set_color(LL_S, RGB_DIM_MAGENTA);
            rgb_matrix_set_color(LL_D, RGB_DIM_MAGENTA);
            // Left and right click
            rgb_matrix_set_color(LL_U, RGB_GREEN);
            rgb_matrix_set_color(LL_O, RGB_GREEN);
            // Mouse movement
            rgb_matrix_set_color(LL_I, RGB_BLUE);
            rgb_matrix_set_color(LL_J, RGB_BLUE);
            rgb_matrix_set_color(LL_K, RGB_BLUE);
            rgb_matrix_set_color(LL_L, RGB_BLUE);
            break;
        case _spcfn:
            // Macos lock screen and Delete
            rgb_matrix_set_color(LL_ESCAPE, RGB_RED);
            rgb_matrix_set_color(LL_BSPACE, RGB_RED);
            // Insert
            rgb_matrix_set_color(LL_BSLASH, RGB_DIM_CYAN);
            // Page Up and Page Down
            rgb_matrix_set_color(LL_U, RGB_DIM_MAGENTA);
            rgb_matrix_set_color(LL_O, RGB_DIM_MAGENTA);
            // Home and End
            rgb_matrix_set_color(LL_H, RGB_GREEN);
            rgb_matrix_set_color(LL_SCOLON, RGB_GREEN);
            // Arrows
            rgb_matrix_set_color(LL_I, RGB_BLUE);
            rgb_matrix_set_color(LL_J, RGB_BLUE);
            rgb_matrix_set_color(LL_K, RGB_BLUE);
            rgb_matrix_set_color(LL_L, RGB_BLUE);
            // Print Screen, Scroll Lock and Pause
            rgb_matrix_set_color(LL_P, RGB_DIM_YELLOW);
            rgb_matrix_set_color(LL_LBRACKET, RGB_DIM_YELLOW);
            rgb_matrix_set_color(LL_RBRACKET, RGB_DIM_YELLOW);
            // _spcfn layer key
            rgb_matrix_set_color(LL_SPACE, RGB_WHITE);
            break;
        case _fn:
            // Brightness up / down
            rgb_matrix_set_color(LL_LBRACKET, RGB_DIM_YELLOW);
            rgb_matrix_set_color(LL_RBRACKET, RGB_DIM_YELLOW);
            // RESET key
            rgb_matrix_set_color(LL_BSLASH, RGB_RED);
            // NKRO toggle
            rgb_matrix_set_toggle(LL_N, keymap_config.nkro);
            // Mouse layer toggle
            rgb_matrix_set_toggle(LL_M, layer_state_is(_mouse));
            // Recording enabled toggle
            rgb_matrix_set_toggle(LL_MENU, user_config.recording_enabled);
            // WPM enabled toggle
            rgb_matrix_set_toggle(LL_W, user_config.wpm_enabled);
            // Play/Pause and Next
            rgb_matrix_set_color(LL_LSHIFT, RGB_DIM_CYAN);
            rgb_matrix_set_color(LL_RSHIFT, RGB_DIM_CYAN);
            // Volume keys
            rgb_matrix_set_color(LL_Z, RGB_DIM_YELLOW);
            rgb_matrix_set_color(LL_X, RGB_DIM_YELLOW);
            rgb_matrix_set_color(LL_C, RGB_DIM_YELLOW);
            // Space key
            rgb_matrix_set_color(LL_SPACE, RGB_BLUE);
            // _fn layer key
            rgb_matrix_set_color(LL_FN, RGB_DIM_WHITE);
            break;
#ifdef DEBUG_LAYER
        case _debug:
            // DB_TOGG
            rgb_matrix_set_toggle(LL_D, debug_enable);
            // DB_MTRX
            rgb_matrix_set_toggle(LL_X, debug_matrix);
            // DB_KBRD
            rgb_matrix_set_toggle(LL_K, debug_keyboard);
            // DB_MOUS
            rgb_matrix_set_toggle(LL_M, debug_mouse);
            // PRT_VER
            rgb_matrix_set_color(LL_V, RGB_BLUE);
            // CLR_EEP
            rgb_matrix_set_color(LL_E, RGB_RED);
            break;
#endif
    }

    led_t led_state = host_keyboard_led_state();
    if (led_state.caps_lock) {
        rgb_matrix_set_color(LL_CAPSLOCK, RGB_WHITE);
    }
}
