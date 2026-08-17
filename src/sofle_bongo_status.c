/*
 * Landscape Bongo Cat status screen for a 128x32 monochrome OLED.
 *
 * Inspired by the responsive Bongo Cat widget from zmk-nice-oled:
 * https://github.com/mctechnology17/zmk-nice-oled
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <string.h>

#include <zephyr/kernel.h>

#include <lvgl.h>

#include <zmk/display.h>
#include <zmk/display/status_screen.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>

#define OLED_WIDTH 128
#define OLED_HEIGHT 32
#define OLED_STRIDE (OLED_WIDTH / 8)
#define PALETTE_SIZE 8
#define TAP_HOLD_MS 360
#define ANIMATION_TICK_MS 250
#define BLINK_TICKS 16

enum bongo_frame { BONGO_IDLE, BONGO_BLINK, BONGO_TAP_LEFT, BONGO_TAP_RIGHT };

static uint8_t image_buffer[PALETTE_SIZE + OLED_STRIDE * OLED_HEIGHT];
static lv_obj_t *image_obj;
static uint32_t last_tap_at;
static uint8_t idle_ticks;
static bool next_paw_right;

static lv_image_dsc_t image = {
    .header =
        {
            .magic = LV_IMAGE_HEADER_MAGIC,
            .cf = LV_COLOR_FORMAT_I1,
            .w = OLED_WIDTH,
            .h = OLED_HEIGHT,
            .stride = OLED_STRIDE,
        },
    .data_size = sizeof(image_buffer),
    .data = image_buffer,
};

static void set_pixel(int x, int y) {
    if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT) {
        return;
    }

    image_buffer[PALETTE_SIZE + y * OLED_STRIDE + x / 8] |= BIT(7 - (x % 8));
}

static void draw_line(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;

    while (true) {
        set_pixel(x0, y0);
        if (x0 == x1 && y0 == y1) {
            break;
        }

        int twice_error = 2 * error;
        if (twice_error >= dy) {
            error += dy;
            x0 += sx;
        }
        if (twice_error <= dx) {
            error += dx;
            y0 += sy;
        }
    }
}

static void draw_disc(int center_x, int center_y, int radius) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                set_pixel(center_x + x, center_y + y);
            }
        }
    }
}

static void draw_drum(int left, int right) {
    int center = (left + right) / 2;

    draw_line(left + 3, 21, right - 3, 21);
    draw_line(left, 23, left + 3, 21);
    draw_line(right, 23, right - 3, 21);
    draw_line(left, 23, left + 3, 26);
    draw_line(right, 23, right - 3, 26);
    draw_line(left + 3, 26, right - 3, 26);
    draw_line(left + 3, 26, left + 5, 31);
    draw_line(right - 3, 26, right - 5, 31);
    draw_line(left + 5, 31, right - 5, 31);
    draw_line(center - 5, 23, center + 5, 23);
}

static void draw_face(bool blinking) {
    /* Ears and head. */
    draw_line(47, 16, 47, 7);
    draw_line(47, 7, 53, 2);
    draw_line(53, 2, 58, 7);
    draw_line(58, 7, 70, 7);
    draw_line(70, 7, 75, 2);
    draw_line(75, 2, 81, 7);
    draw_line(81, 7, 81, 16);
    draw_line(47, 16, 51, 20);
    draw_line(51, 20, 77, 20);
    draw_line(77, 20, 81, 16);

    if (blinking) {
        draw_line(56, 12, 60, 12);
        draw_line(69, 12, 73, 12);
    } else {
        draw_disc(58, 12, 1);
        draw_disc(71, 12, 1);
    }

    set_pixel(64, 15);
    draw_line(61, 17, 64, 19);
    draw_line(64, 19, 67, 17);

    /* Whiskers. */
    draw_line(53, 15, 44, 13);
    draw_line(53, 17, 43, 18);
    draw_line(76, 15, 85, 13);
    draw_line(76, 17, 86, 18);
}

static void draw_resting_paws(void) {
    draw_line(56, 20, 43, 20);
    draw_line(43, 20, 35, 23);
    draw_disc(35, 23, 3);
    draw_line(73, 20, 86, 20);
    draw_line(86, 20, 94, 23);
    draw_disc(94, 23, 3);
}

static void draw_tapping_paws(bool right_paw) {
    if (right_paw) {
        draw_line(73, 20, 86, 20);
        draw_line(86, 20, 94, 23);
        draw_disc(94, 23, 3);

        draw_line(56, 20, 46, 15);
        draw_line(46, 15, 38, 12);
        draw_disc(38, 12, 3);
        draw_line(27, 16, 27, 11);
        draw_line(25, 17, 22, 14);
    } else {
        draw_line(56, 20, 43, 20);
        draw_line(43, 20, 35, 23);
        draw_disc(35, 23, 3);

        draw_line(73, 20, 83, 15);
        draw_line(83, 15, 91, 12);
        draw_disc(91, 12, 3);
        draw_line(101, 16, 101, 11);
        draw_line(103, 17, 106, 14);
    }
}

static void draw_frame(enum bongo_frame frame) {
    /* Indexed 1-bit palette: index 0 is black, index 1 is white. */
    image_buffer[0] = 0x00;
    image_buffer[1] = 0x00;
    image_buffer[2] = 0x00;
    image_buffer[3] = 0xff;
    image_buffer[4] = 0xff;
    image_buffer[5] = 0xff;
    image_buffer[6] = 0xff;
    image_buffer[7] = 0xff;
    memset(&image_buffer[PALETTE_SIZE], 0, sizeof(image_buffer) - PALETTE_SIZE);

    draw_line(8, 25, 120, 25);
    draw_drum(20, 48);
    draw_drum(80, 108);
    draw_face(frame == BONGO_BLINK);
    draw_line(53, 20, 51, 25);
    draw_line(76, 20, 78, 25);

    if (frame == BONGO_TAP_LEFT) {
        draw_tapping_paws(false);
    } else if (frame == BONGO_TAP_RIGHT) {
        draw_tapping_paws(true);
    } else {
        draw_resting_paws();
    }

    if (image_obj != NULL) {
        lv_obj_invalidate(image_obj);
    }
}

struct bongo_input_state {
    bool pressed;
};

static struct bongo_input_state bongo_input_get_state(const zmk_event_t *event) {
    const struct zmk_position_state_changed *position_event =
        event != NULL ? as_zmk_position_state_changed(event) : NULL;

    return (struct bongo_input_state){
        .pressed = position_event != NULL && position_event->state,
    };
}

static void bongo_input_update_cb(struct bongo_input_state state) {
    if (!state.pressed || image_obj == NULL) {
        return;
    }

    last_tap_at = k_uptime_get_32();
    next_paw_right = !next_paw_right;
    draw_frame(next_paw_right ? BONGO_TAP_RIGHT : BONGO_TAP_LEFT);
}

ZMK_DISPLAY_WIDGET_LISTENER(sofle_bongo_input, struct bongo_input_state, bongo_input_update_cb,
                            bongo_input_get_state)
ZMK_SUBSCRIPTION(sofle_bongo_input, zmk_position_state_changed);

static void animation_timer_cb(lv_timer_t *timer) {
    ARG_UNUSED(timer);

    uint32_t now = k_uptime_get_32();
    if (last_tap_at != 0 && now - last_tap_at < TAP_HOLD_MS) {
        return;
    }

    idle_ticks = (idle_ticks + 1) % BLINK_TICKS;
    draw_frame(idle_ticks == 0 ? BONGO_BLINK : BONGO_IDLE);
}

lv_obj_t *zmk_display_status_screen(void) {
    lv_obj_t *screen = lv_obj_create(NULL);

    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(screen, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(screen, 0, LV_PART_MAIN);

    draw_frame(BONGO_IDLE);
    image_obj = lv_image_create(screen);
    lv_image_set_src(image_obj, &image);
    lv_obj_align(image_obj, LV_ALIGN_CENTER, 0, 0);

    sofle_bongo_input_init();
    lv_timer_create(animation_timer_cb, ANIMATION_TICK_MS, NULL);

    return screen;
}
