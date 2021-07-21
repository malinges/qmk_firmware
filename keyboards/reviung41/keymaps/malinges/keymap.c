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
#include <stdio.h>

#ifndef MIN
#  define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#  define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

enum layer_names {
  _BASE,
  _LOWER,
  _RAISE,
  _ADJUST
};

enum custom_keycodes {
  OLED_TOG = SAFE_RANGE,  // Toggle OLED on/off
};

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

#ifdef RGBLIGHT_ENABLE
#  ifndef RGB_TUNING_KEYCODE_REPEAT_INTERVAL
#    define RGB_TUNING_KEYCODE_REPEAT_INTERVAL TAPPING_TERM
#  endif
static uint16_t rgb_keycode;
static keyrecord_t rgb_record;
static uint16_t rgb_timer;
#  ifdef OLED_DRIVER_ENABLE
#    ifndef RGB_KEYCODE_OLED_DISPLAY_TIME
#     define RGB_KEYCODE_OLED_DISPLAY_TIME 2000
#    endif
extern rgblight_config_t rgblight_config;
static bool oled_show_rgb_kc = false;
#  endif
#endif

#ifdef OLED_DRIVER_ENABLE

typedef union {
  uint32_t raw;
  struct {
    bool     oled_enable:1;
  };
} user_config_t;

static user_config_t user_config;

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

void keyboard_post_init_user(void) {
  user_config.raw = eeconfig_read_user();
  oled_enable(user_config.oled_enable);
}

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
  return OLED_ROTATION_180;
}

#  ifdef RGBLIGHT_ENABLE

static void oled_write_uint16(uint16_t i, uint8_t min_width) {
  char buf[6];
  {
    char format[6];
    sprintf(format, "%%%uu", MIN(min_width, 5));
    sprintf(buf, format, i);
  }
  oled_write(buf, false);
}

static bool oled_write_rgb_mode_name(uint8_t mode_base, const char *data, bool multiple) {
  if (rgblight_config.mode < mode_base) {
    return false;
  }

  oled_write_P(data, false);
  if (multiple) {
    oled_write_char(' ', false);
    oled_write_uint16(rgblight_config.mode - mode_base + 1, 1);
  }
  oled_write_ln("", false);

  return true;
}

#  endif // RGBLIGHT_ENABLE

void oled_task_user(void) {
  if (!is_oled_enabled()) {
    return;
  }

#  ifdef RGBLIGHT_ENABLE

  bool oled_write_2_ln = true;

  if (oled_show_rgb_kc) {
    if (rgb_record.event.pressed || timer_elapsed(rgb_timer) < RGB_KEYCODE_OLED_DISPLAY_TIME) {
      switch (rgb_keycode) {
        case RGB_TOG:
          oled_write_P(PSTR("RGB: "), false);
          oled_write_P(rgblight_config.enable ? PSTR("on") : PSTR("off"), false);
          break;
        case RGB_MODE_FORWARD...RGB_MODE_REVERSE:
          oled_write_ln_P(PSTR("RGB mode:"), false);
          if (!(
#    ifdef RGBLIGHT_EFFECT_TWINKLE
            oled_write_rgb_mode_name(RGBLIGHT_MODE_TWINKLE, PSTR("Twinkle"), true) ||
#    endif
#    ifdef RGBLIGHT_EFFECT_ALTERNATING
            oled_write_rgb_mode_name(RGBLIGHT_MODE_ALTERNATING, PSTR("Alternating"), false) ||
#    endif
#    ifdef RGBLIGHT_EFFECT_RGB_TEST
            oled_write_rgb_mode_name(RGBLIGHT_MODE_RGB_TEST, PSTR("RGB test"), false) ||
#    endif
#    ifdef RGBLIGHT_EFFECT_STATIC_GRADIENT
            oled_write_rgb_mode_name(RGBLIGHT_MODE_STATIC_GRADIENT, PSTR("Static gradient"), true) ||
#    endif
#    ifdef RGBLIGHT_EFFECT_CHRISTMAS
            oled_write_rgb_mode_name(RGBLIGHT_MODE_CHRISTMAS, PSTR("Christmas"), false) ||
#    endif
#    ifdef RGBLIGHT_EFFECT_KNIGHT
            oled_write_rgb_mode_name(RGBLIGHT_MODE_KNIGHT, PSTR("Knight"), true) ||
#    endif
#    ifdef RGBLIGHT_EFFECT_SNAKE
            oled_write_rgb_mode_name(RGBLIGHT_MODE_SNAKE, PSTR("Snake"), true) ||
#    endif
#    ifdef RGBLIGHT_EFFECT_RAINBOW_SWIRL
            oled_write_rgb_mode_name(RGBLIGHT_MODE_RAINBOW_SWIRL, PSTR("Rainbow swirl"), true) ||
#    endif
#    ifdef RGBLIGHT_EFFECT_RAINBOW_MOOD
            oled_write_rgb_mode_name(RGBLIGHT_MODE_RAINBOW_MOOD, PSTR("Rainbow mood"), true) ||
#    endif
#    ifdef RGBLIGHT_EFFECT_BREATHING
            oled_write_rgb_mode_name(RGBLIGHT_MODE_BREATHING, PSTR("Breathing"), true) ||
#    endif
            false)) {
            oled_write_ln_P(PSTR("Static light"), false);
          }
          oled_write_2_ln = false;
          break;
        case RGB_HUI...RGB_HUD:
          oled_write_P(PSTR("RGB hue: "), false);
          oled_write_uint16(((uint32_t)360) * rgblight_config.hue / 256, 3);
          oled_write(" deg", false);
          break;
        case RGB_SAI...RGB_SAD:
          oled_write_P(PSTR("RGB saturation: "), false);
          oled_write_uint16(((uint16_t)100) * rgblight_config.sat / 255, 3);
          oled_write_char('%', false);
          break;
        case RGB_VAI...RGB_VAD:
          oled_write_P(PSTR("RGB brightness: "), false);
          oled_write_uint16(((uint16_t)100) * rgblight_config.val / RGBLIGHT_LIMIT_VAL, 3);
          oled_write_char('%', false);
          break;
        // RGB Lighting subsystem doesn't really use these keycodes, uncomment when using RGB Matrix
        // case RGB_SPI...RGB_SPD:
        //   oled_write_P(PSTR("RGB speed: "), false);
        //   oled_write_uint16(rgblight_config.speed, 1);
        //   break;
      }
    } else {
      oled_show_rgb_kc = false;
    }
  }

  if (oled_write_2_ln) {
    oled_write_ln("", false);
    oled_write_ln("", false);
  }

#  endif // RGBLIGHT_ENABLE

  // Host Keyboard LED Status
  led_t led_state = host_keyboard_led_state();
  oled_write_P(led_state.num_lock ? PSTR("NUM ") : PSTR("    "), false);
  oled_write_P(led_state.caps_lock ? PSTR("CAP ") : PSTR("    "), false);
  oled_write_P(led_state.scroll_lock ? PSTR("SCR") : PSTR("   "), false);

#  ifdef OLED_SHOW_FPS
  static uint16_t fps = 0;
  static uint16_t counter = 0;
  static uint16_t start = 0;

  uint16_t now = timer_read();

  if (now - start >= 1000) {
    fps = ((uint32_t)counter) * (now - start) / 1000;
    counter = 1;
    start = now;
  } else {
    counter++;
  }

  oled_set_cursor(oled_max_chars() - 3, 0);
  oled_write_uint16(fps, 3);
#  endif
}

#endif // OLED_DRIVER_ENABLE

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
#ifdef RGBLIGHT_ENABLE
    case RGB_TOG...RGB_MODE_RGBTEST:
      rgb_keycode = keycode;
      rgb_record = *record;
      rgb_timer = timer_read();
#  ifdef OLED_DRIVER_ENABLE
      oled_show_rgb_kc = true;
#  endif
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
  if (rgb_keycode >= RGB_HUI && rgb_keycode <= RGB_SPD && rgb_record.event.pressed && timer_elapsed(rgb_timer) >= RGB_TUNING_KEYCODE_REPEAT_INTERVAL) {
    rgb_timer += RGB_TUNING_KEYCODE_REPEAT_INTERVAL;
    process_rgb(rgb_keycode, &rgb_record);
  }
#endif
}
