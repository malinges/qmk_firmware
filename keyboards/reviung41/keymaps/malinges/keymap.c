/* Copyright 2020 gtips
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

enum layer_names {
  _BASE,
  _LOWER,
  _RAISE,
  _ADJUST
};

enum custom_keycodes {
  OLED_TOG = SAFE_RANGE,  // Toggle OLED on/off
};

typedef union {
  uint32_t raw;
  struct {
    bool     oled_enable:1;
  };
} user_config_t;

static user_config_t user_config;

#define LOWER  MO(_LOWER)
#define RAISE  MO(_RAISE)
#define ADJUST MO(_ADJUST)

#define ALT_ESC LALT_T(KC_ESC)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [_BASE] = LAYOUT_reviung41(
    KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,      KC_T,               KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_BSPC,
    ALT_ESC,  KC_A,     KC_S,     KC_D,     KC_F,      KC_G,               KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,
    KC_LCTL,  KC_Z,     KC_X,     KC_C,     KC_V,      KC_B,               KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,  KC_RCTL,
                                            KC_LGUI,   LOWER,    KC_SPC,   RAISE,    RSFT_T(KC_ENT)
  ),

  [_LOWER] = LAYOUT_reviung41(
    KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,      KC_5,               KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_DEL,
    _______,  KC_MINS,  KC_EQL,   KC_LBRC,  KC_RBRC,   KC_BSLS,            KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,
    _______,  KC_ESC,   KC_LGUI,  KC_LALT,  KC_CAPS,   KC_QUOT,            KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,
                                            _______,   _______,  KC_BSPC,  _______,  _______
  ),

  [_RAISE] = LAYOUT_reviung41(
    KC_TILD,  KC_EXLM,  KC_AT,    KC_HASH,  KC_DLR,    KC_PERC,            KC_CIRC,  KC_AMPR,  KC_ASTR,  KC_LPRN,  KC_RPRN,  KC_DEL,
    _______,  KC_UNDS,  KC_PLUS,  KC_LCBR,  KC_RCBR,   KC_PIPE,            KC_LEFT,  KC_DOWN,  KC_UP,    KC_RGHT,  KC_GRV,   KC_TILD,
    _______,  KC_ESC,   KC_RGUI,  KC_RALT,  KC_CAPS,   KC_DQUO,            KC_HOME,  KC_END,   KC_PGUP,  KC_PGDN,  KC_PSCR,  RSFT_T(KC_SPC),
                                            _______,   _______,  KC_ENT,   _______,  _______
  ),

  [_ADJUST] = LAYOUT_reviung41(
    RGB_VAI,   RGB_SAI, RGB_HUI,  RGB_MOD,  XXXXXXX,   RGB_TOG,            OLED_TOG, KC_VOLD,  KC_MUTE,  KC_VOLU,  KC_BRID,  KC_BRIU,
    RGB_VAD,   RGB_SAD, RGB_HUD,  RGB_RMOD, XXXXXXX,   XXXXXXX,            XXXXXXX,  KC_MPRV,  KC_MPLY,  KC_MNXT,  XXXXXXX,  XXXXXXX,
    XXXXXXX,   XXXXXXX, XXXXXXX,  XXXXXXX,  XXXXXXX,   XXXXXXX,            RESET,    XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,
                                            _______,   _______,  XXXXXXX,  _______,  _______
  ),
};

layer_state_t layer_state_set_user(layer_state_t state) {
  return update_tri_layer_state(state, _LOWER, _RAISE, _ADJUST);
}

#ifdef OLED_DRIVER_ENABLE
static bool is_oled_enabled(void) {
  return user_config.oled_enable;
}

static void oled_enable(bool enable) {
  user_config.oled_enable = enable;
  eeconfig_update_user(user_config.raw);
  if (!enable) {
    oled_clear();
  }
}

static void oled_toggle(void) {
  oled_enable(!is_oled_enabled());
}

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
  return OLED_ROTATION_180;
}

void oled_task_user(void) {
  if (!is_oled_enabled()) {
    return;
  }

  oled_write_P(PSTR("Layer: "), false);

  switch (get_highest_layer(layer_state)) {
    case _BASE:
      oled_write_ln_P(PSTR("base"), false);
      break;
    case _LOWER:
      oled_write_ln_P(PSTR("LOWER"), false);
      break;
    case _RAISE:
      oled_write_ln_P(PSTR("RAISE"), false);
      break;
    case _ADJUST:
      oled_write_ln_P(PSTR("ADJUST"), false);
      break;
    default:
      // Or use the write_ln shortcut over adding '\n' to the end of your string
      oled_write_ln_P(PSTR("Unknown?!"), false);
  }

  // Host Keyboard LED Status
  led_t led_state = host_keyboard_led_state();
  oled_write_P(led_state.num_lock ? PSTR("NUM ") : PSTR("    "), false);
  oled_write_P(led_state.caps_lock ? PSTR("CAP ") : PSTR("    "), false);
  oled_write_P(led_state.scroll_lock ? PSTR("SCR ") : PSTR("    "), false);
}
#endif

void keyboard_post_init_user(void) {
  user_config.raw = eeconfig_read_user();
#ifdef OLED_DRIVER_ENABLE
  oled_enable(user_config.oled_enable);
#endif
}

#ifdef RGBLIGHT_ENABLE
#  ifndef RGB_TUNING_KEYCODE_REPEAT_INTERVAL
#    define RGB_TUNING_KEYCODE_REPEAT_INTERVAL TAPPING_TERM
#  endif

static uint16_t rgb_keycode;
static keyrecord_t rgb_record;
static uint16_t rgb_timer;
#endif

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
#ifdef RGBLIGHT_ENABLE
    case RGB_HUI...RGB_SPD:
      rgb_keycode = keycode;
      rgb_record = *record;
      rgb_timer = timer_read();
      return true;
#endif
    case OLED_TOG:
#ifdef OLED_DRIVER_ENABLE
      if (record->event.pressed) {
        oled_toggle();
      }
#endif
      return false;
    default:
      return true; // Process all other keycodes normally
  }
}

void housekeeping_task_user(void) {
#ifdef RGBLIGHT_ENABLE
  if (rgb_record.event.pressed && timer_elapsed(rgb_timer) >= RGB_TUNING_KEYCODE_REPEAT_INTERVAL) {
    rgb_timer += RGB_TUNING_KEYCODE_REPEAT_INTERVAL;
    process_rgb(rgb_keycode, &rgb_record);
  }
#endif
}
