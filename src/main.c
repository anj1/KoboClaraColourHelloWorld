#define _GNU_SOURCE

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fbink.h"

static volatile sig_atomic_t running = 1;

static void on_signal(int sig) {
    (void)sig;
    running = 0;
}

// Helper function to draw a bounds-checked filled circle using packed pixels
static void draw_filled_circle(int fbfd, uint32_t screen_width, uint32_t screen_height, int32_t cx, int32_t cy, int32_t r, uint32_t packed_color) {
    int32_t r2 = r * r;
    // Define the safe vertical boundaries
    int32_t min_y = 106;
    int32_t max_y = (int32_t)screen_height - 106;

    for (int32_t dy = -r; dy <= r; dy++) {
        int32_t py = cy + dy;
        // Restrict drawing to the area between the top and bottom status bars
        if (py < min_y || py >= max_y) {
            continue;
        }
        for (int32_t dx = -r; dx <= r; dx++) {
            int32_t px = cx + dx;
            if (px < 0 || px >= (int32_t)screen_width) {
                continue;
            }
            if (dx * dx + dy * dy <= r2) {
                fbink_put_pixel(fbfd, (uint16_t)px, (uint16_t)py, &packed_color);
            }
        }
    }
}
static void draw_home(int fbfd) {
    FBInkConfig cfg = {0};

    cfg.is_quiet = true;
    cfg.is_centered = true;
    cfg.fontmult = 2;
    cfg.is_cleared = true;
    cfg.no_refresh = true; 

    if (fbink_init(fbfd, &cfg) < 0) {
        fprintf(stderr, "fbink_init failed\n");
        return;
    }

    FBInkState state;
    fbink_get_state(&cfg, &state);
    uint32_t width = state.screen_width;
    uint32_t height = state.screen_height;

    // Clear only your custom viewport region (y=106 to y=height-106)
    FBInkRect clear_rect = {
        .left = 0,
        .top = 106,
        .width = width,
        .height = height - 212
    };
    fbink_cls(fbfd, &cfg, &clear_rect, false);

    cfg.is_cleared = false;

    // Calculate how many rows of text fit into our 106px margin zone.
    // state.font_h is the actual height of a text row at the current fontmult.
    int16_t top_margin_rows = 106 / state.font_h;
    // Calculate the remaining pixel remainder if 106 isn't perfectly divisible by font_h
    int16_t top_remainder = 106 % state.font_h;

    // Draw the text header (Add 4 rows to our top margin boundary)
    cfg.row = top_margin_rows + 4;
    cfg.voffset = top_remainder; // Add the pixel remainder to align perfectly with y=106
    fbink_print(fbfd, "My App", &cfg);

    // Draw the subheader 2 rows further down
    cfg.row = top_margin_rows + 6;
    fbink_print(fbfd, "Framebuffer app running via FBInk", &cfg);

    // Calculate dimensions for the circles
    uint32_t usable_height = height - 212; 
    uint32_t min_dim = (width < usable_height) ? width : usable_height;
    int32_t radius = min_dim * 0.10; 

    // Center coordinates for three horizontal circles inside the safe area
    int32_t cy = 106 + (usable_height / 2);
    int32_t cx_red = width / 4;
    int32_t cx_green = width / 2;
    int32_t cx_blue = (3 * width) / 4;

    uint32_t red_pixel = 0;
    uint32_t green_pixel = 0;
    uint32_t blue_pixel = 0;

    fbink_pack_pixel_rgba(255, 0, 0, 255, &red_pixel);
    fbink_pack_pixel_rgba(0, 255, 0, 255, &green_pixel);
    fbink_pack_pixel_rgba(0, 0, 255, 255, &blue_pixel);

    draw_filled_circle(fbfd, width, height, cx_red, cy, radius, red_pixel);
    draw_filled_circle(fbfd, width, height, cx_green, cy, radius, green_pixel);
    draw_filled_circle(fbfd, width, height, cx_blue, cy, radius, blue_pixel);

    // Update state to use a font multiplier of 1 for the footer text
    cfg.fontmult = 1;
    fbink_init(fbfd, &cfg);
    fbink_get_state(&cfg, &state);

    // Calculate how many rows of text fit into the bottom 106px margin zone
    int16_t bottom_margin_rows = 106 / state.font_h;
    int16_t bottom_remainder = 106 % state.font_h;

    // Negative rows count backwards from the absolute bottom of the screen.
    // We move up past the bottom margin rows, and up 4 more rows for padding.
    cfg.row = -(bottom_margin_rows + 4);
    cfg.voffset = -bottom_remainder; // Offset backwards by the remainder to stay inside safe area
    fbink_print(fbfd, "Press Stop My App from NickelMenu to exit", &cfg);

    // Trigger a single hardware screen refresh restricted to our modified area
    cfg.no_refresh = false;
    fbink_refresh(fbfd, 106, 0, width, usable_height, &cfg); 
}

int main(void) {
    signal(SIGTERM, on_signal);
    signal(SIGINT, on_signal);

    int fbfd = fbink_open();
    if (fbfd < 0) {
        fprintf(stderr, "fbink_open failed: %s\n", strerror(errno));
        return 1;
    }

    draw_home(fbfd);

    while (running) {
        sleep(1);
    }

    FBInkConfig cfg = {0};
    cfg.is_quiet = true;
    cfg.is_cleared = true;
    fbink_init(fbfd, &cfg);
    fbink_cls(fbfd, &cfg, NULL, false);

    fbink_close(fbfd);
    return 0;
}
