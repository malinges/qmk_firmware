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

#include QMK_KEYBOARD_H
#include "raw_hid.h"

enum layers {
_qwerty,
_gamer,
_spcfn,
_fn
};

enum keycodes {
REC_TOG = SAFE_RANGE,
REN_TOG
};

enum output_messages {
    RECORDING_UPDATE
};

enum input_messages {
    RECORDING_QUERY,
    RECORDING_ACK
};

#define RECORDING_RGB_COLOR RGB_RED

static bool local_recording = false;
static bool remote_recording = false;
static uint16_t rec_tog_timer = 0;

typedef union {
  uint32_t raw;
  struct {
    bool recording_enabled:1;
  };
} user_config_t;

user_config_t user_config;

static void send_recording(void) {
    uint8_t data[RAW_EPSIZE];
    memset(data, 0, sizeof(data));

    data[0] = RECORDING_UPDATE;
    data[1] = local_recording;

    raw_hid_send(data, sizeof(data));
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

#define KC_SPFN LT(_spcfn, KC_SPC) // press for space, hold for function layer (aka spacefn)
#define T_GAMER TG(_gamer)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_qwerty] = LAYOUT_60_ansi(
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS,
        KC_ESC,  KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,          KC_ENT,
        KC_LSFT,          KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,          KC_RSFT,
        KC_LCTL, KC_LGUI, KC_LALT,                            KC_SPFN,                            KC_RALT, REC_TOG, KC_RCTL, MO(_fn)
    ),
    // Gamer standard qwerty layout but with normal space for jumping etc
    [_gamer] = LAYOUT_60_ansi(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______,          _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______, _______, _______,                            KC_SPACE,                           _______, _______, _______, _______
    ),
    [_spcfn] = LAYOUT_60_ansi(
        KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_DEL,
        _______, _______, _______, _______, _______, _______, KC_PGUP, KC_HOME, KC_UP,   KC_END,  KC_PSCR, KC_SLCK, KC_PAUS, KC_INS,
        KC_CAPS, _______, _______, _______, _______, _______, KC_PGDN, KC_LEFT, KC_DOWN, KC_RGHT, _______, _______,          _______,
        _______,          _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______, _______, _______,                            _______,                            _______, _______, _______, _______
    ),
    [_fn] = LAYOUT_60_ansi(
        DM_RSTP, DM_REC1, DM_REC2, DM_PLY1, DM_PLY2, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, RGB_TOG, XXXXXXX, RGB_HUI, RGB_HUD, RGB_SAI, RGB_SAD, RGB_VAI, RGB_VAD, RGB_MOD, RGB_RMOD,XXXXXXX, XXXXXXX, RESET,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, T_GAMER, XXXXXXX, XXXXXXX, RGB_SPI, RGB_SPD, XXXXXXX, XXXXXXX,          XXXXXXX,
        KC_MPLY,          KC_VOLD, KC_VOLU, KC_MUTE, XXXXXXX, XXXXXXX, NK_TOGG, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,          KC_MNXT,
        XXXXXXX, XXXXXXX, XXXXXXX,                            KC_SPC,                             XXXXXXX, REN_TOG, XXXXXXX, _______
    )
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case REC_TOG:
            if (user_config.recording_enabled) {
                if (record->event.pressed) {
                    rec_tog_timer = timer_read();
                    set_recording(!local_recording);
                } else {
                    if (timer_read() - rec_tog_timer > TAPPING_TERM) {
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
        default:
            return true; // Process all other keycodes normally
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
    if (!g_suspend_state && rgb_matrix_config.enable) {
        if (user_config.recording_enabled) {
            if (local_recording) {
                rgb_matrix_set_color(58, RECORDING_RGB_COLOR);
            }
            if (remote_recording) {
                rgb_matrix_layer_helper(RECORDING_RGB_COLOR, LED_FLAG_UNDERGLOW);
            }
        }

        switch (get_highest_layer(layer_state)) {
            case _spcfn:
                // Escape and Delete
                rgb_matrix_set_color(0, 0xFF, 0x00, 0x00);
                rgb_matrix_set_color(13, 0xFF, 0x00, 0x00);
                // Insert
                rgb_matrix_set_color(27, 0x00, 0x7F, 0x7F);
                // Caps Lock
                rgb_matrix_set_color(28, 0x66, 0x66, 0x66);
                // Page Up and Page Down
                rgb_matrix_set_color(20, 0x7F, 0x00, 0x7F);
                rgb_matrix_set_color(34, 0x7F, 0x00, 0x7F);
                // Home and End
                rgb_matrix_set_color(21, 0x00, 0xFF, 0x00);
                rgb_matrix_set_color(23, 0x00, 0xFF, 0x00);
                // Print Screen, Scroll Lock and Pause
                rgb_matrix_set_color(24, 0x7F, 0x7f, 0x00);
                rgb_matrix_set_color(25, 0x7F, 0x7f, 0x00);
                rgb_matrix_set_color(26, 0x7F, 0x7f, 0x00);
                // Arrows
                rgb_matrix_set_color(22, 0x00, 0x00, 0xFF);
                rgb_matrix_set_color(35, 0x00, 0x00, 0xFF);
                rgb_matrix_set_color(36, 0x00, 0x00, 0xFF);
                rgb_matrix_set_color(37, 0x00, 0x00, 0xFF);
                break;
            case _gamer:
                rgb_matrix_set_color(16, 0xFF, 0x00, 0xFF);
                rgb_matrix_set_color(29, 0xFF, 0x00, 0xFF);
                rgb_matrix_set_color(30, 0xFF, 0x00, 0xFF);
                rgb_matrix_set_color(31, 0xFF, 0x00, 0xFF);
                break;
            case _fn:
                // Dynamic macro keys
                rgb_matrix_set_color(0, 0x7f, 0x7f, 0x00);
                rgb_matrix_set_color(1, 0x7f, 0x00, 0x7f);
                rgb_matrix_set_color(2, 0x7f, 0x00, 0x7f);
                rgb_matrix_set_color(3, 0x00, 0x7f, 0x7f);
                rgb_matrix_set_color(4, 0x00, 0x7f, 0x7f);
                // NKRO toggle
                if (keymap_config.nkro) {
                    rgb_matrix_set_color(47, 0x00, 0xFF, 0x00);
                } else {
                    rgb_matrix_set_color(47, 0xFF, 0x00, 0x00);
                }
                // Gamer layer toggle
                if (layer_state_is(_gamer)) {
                    rgb_matrix_set_color(33, 0x00, 0xFF, 0x00);
                } else {
                    rgb_matrix_set_color(33, 0xFF, 0x00, 0x00);
                }
                // Recording enabled toggle
                if (user_config.recording_enabled) {
                    rgb_matrix_set_color(58, RGB_GREEN);
                } else {
                    rgb_matrix_set_color(58, RGB_RED);
                }
                // Play/Pause and Next
                rgb_matrix_set_color(41, 0x00, 0x7F, 0x7F);
                rgb_matrix_set_color(52, 0x00, 0x7F, 0x7F);
                // Volume keys
                rgb_matrix_set_color(42, 0x7F, 0x7F, 0x00);
                rgb_matrix_set_color(43, 0x7F, 0x7F, 0x00);
                rgb_matrix_set_color(44, 0x7F, 0x7F, 0x00);
                // Space key
                rgb_matrix_set_color(56, RGB_BLUE);
                // _fn layer key
                rgb_matrix_set_color(60, 0x7F, 0x7F, 0x7F);
                break;
        }

        led_t led_state = host_keyboard_led_state();
        if (led_state.caps_lock) {
            rgb_matrix_set_color(28, 0xFF, 0xFF, 0xFF);
        }
    }
}
