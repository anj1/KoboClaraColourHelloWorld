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
    for (int32_t dy = -r; dy <= r; dy++) {
        int32_t py = cy + dy;
        if (py < 0 || py >= (int32_t)screen_height) {
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
    cfg.no_refresh = true; // Handle refreshing manually after drawing is complete

    if (fbink_init(fbfd, &cfg) < 0) {
        fprintf(stderr, "fbink_init failed\n");
        return;
    }

    // Clear the screen buffer without refreshing yet
    fbink_cls(fbfd, &cfg, NULL, false);

    cfg.is_cleared = false;

    // Draw the text header
    cfg.row = 4;
    fbink_print(fbfd, "My App", &cfg);

    cfg.row = 8;
    cfg.fontmult = 1;
    fbink_print(fbfd, "Framebuffer app running via FBInk", &cfg);

    // Get screen dimensions to calculate sizes and positions
    FBInkState state;
    fbink_get_state(&cfg, &state);
    uint32_t width = state.screen_width;
    uint32_t height = state.screen_height;

    // Calculate dimensions for the circles (20% of the screen's smaller dimension)
    uint32_t min_dim = (width < height) ? width : height;
    int32_t radius = min_dim * 0.10; // Radius is 10% (Diameter is 20%)

    // Center coordinates for three horizontal circles
    int32_t cy = height / 2;
    int32_t cx_red = width / 4;
    int32_t cx_green = width / 2;
    int32_t cx_blue = (3 * width) / 4;

    // Pack RGB colors according to the current framebuffer pixel format
    uint32_t red_pixel = 0;
    uint32_t green_pixel = 0;
    uint32_t blue_pixel = 0;

    fbink_pack_pixel_rgba(255, 0, 0, 255, &red_pixel);
    fbink_pack_pixel_rgba(0, 255, 0, 255, &green_pixel);
    fbink_pack_pixel_rgba(0, 0, 255, 255, &blue_pixel);

    // Draw the circles into the framebuffer
    draw_filled_circle(fbfd, width, height, cx_red, cy, radius, red_pixel);
    draw_filled_circle(fbfd, width, height, cx_green, cy, radius, green_pixel);
    draw_filled_circle(fbfd, width, height, cx_blue, cy, radius, blue_pixel);

    // Draw the footer text
    cfg.row = -4;
    fbink_print(fbfd, "Press Stop My App from NickelMenu to exit", &cfg);

    // Trigger a single refresh to show all modifications on screen
    cfg.no_refresh = false;
    fbink_refresh(fbfd, 0, 0, 0, 0, &cfg);
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