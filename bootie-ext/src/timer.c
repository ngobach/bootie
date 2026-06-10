#include <bootie.h>
#include <bootie-gfx.h>
#include <stdint.h>

#define TICK_MS     10
#define BORDER_T    3
#define WND_W       380
#define WND_H       200

int gmain(int argc, char *argv[], int flags) {
    (void)argc;
    (void)argv;
    (void)flags;

    struct gfx g;
    if (!gfx_init(&g)) return 0;
    gfx_font_load();

    uint32_t W = gfx_width(&g);
    uint32_t H = gfx_height(&g);

    int wnd_l = (W - WND_W) / 2;
    int wnd_t = (H - WND_H) / 2;
    int wnd_r = wnd_l + WND_W;
    int wnd_b = wnd_t + WND_H;

    int elapsed_ms = 0;
    int running = 0;
    int exit_req = 0;

    while (!exit_req) {
        /* Input */
        while (gfx_checkkey(&g)) {
            int key = gfx_getkey(&g);
            if (key == 27) {
                if (running) {
                    elapsed_ms = 0;
                    running = 0;
                } else {
                    exit_req = 1;
                }
            }
            if (key == ' ') {
                running = !running;
            }
            if (key == 'r' || key == 'R') {
                elapsed_ms = 0;
                running = 0;
            }
        }

        /* Advance timer */
        if (running) {
            elapsed_ms += TICK_MS;
        }

        /* --- Rendering --- */
        gfx_backbuffer_begin(&g);
        fill_rect(&g, 0, 0, W, H, 10, 10, 15);

        /* Window background */
        fill_rect(&g, wnd_l, wnd_t, WND_W, WND_H, 20, 20, 30);

        /* Border */
        fill_rect(&g, wnd_l - BORDER_T, wnd_t - BORDER_T,
                  WND_W + BORDER_T * 2, BORDER_T, 100, 100, 150);
        fill_rect(&g, wnd_l - BORDER_T, wnd_b,
                  WND_W + BORDER_T * 2, BORDER_T, 100, 100, 150);
        fill_rect(&g, wnd_l - BORDER_T, wnd_t - BORDER_T, BORDER_T,
                  WND_H + BORDER_T * 2, 100, 100, 150);
        fill_rect(&g, wnd_r, wnd_t - BORDER_T, BORDER_T,
                  WND_H + BORDER_T * 2, 100, 100, 150);

        /* Format & draw elapsed time */
        int centis = (elapsed_ms / 10) % 100;
        int secs = (elapsed_ms / 1000) % 60;
        int mins = (elapsed_ms / 60000) % 60;
        int hrs = elapsed_ms / 3600000;

        int px_size = 28;
        int tx = (wnd_l + wnd_r - gfx_text_width("00:00:00.00", px_size)) / 2;
        int ty = (wnd_t + wnd_b - px_size) / 2 - 10;
        draw_strf(&g, tx, ty, 0, 220, 220, px_size,
                  "%02d:%02d:%02d.%02d", hrs, mins, secs, centis);

        /* State indicator */
        const char *state;
        int state_r, state_g, state_b;
        if (!running && elapsed_ms == 0) {
            state = "READY";
            state_r = 200; state_g = 200; state_b = 200;
        } else if (running) {
            state = "RUNNING";
            state_r = 0; state_g = 220; state_b = 0;
        } else {
            state = "PAUSED";
            state_r = 220; state_g = 220; state_b = 0;
        }
        draw_str(&g, (wnd_l + wnd_r - gfx_text_width(state, 16)) / 2,
                 ty + px_size + 8, state, state_r, state_g, state_b, 16);

        /* Controls */
        const char *help1 = "SPACE: Start/Stop  R: Reset";
        const char *help2 = "ESC: Reset when running, Exit when stopped";
        draw_str(&g, (wnd_l + wnd_r - gfx_text_width(help1, 16)) / 2,
                 wnd_b - 34, help1, 150, 150, 160, 16);
        draw_str(&g, (wnd_l + wnd_r - gfx_text_width(help2, 16)) / 2,
                 wnd_b - 20, help2, 150, 150, 160, 16);

        gfx_backbuffer_end(&g);
        gfx_delay_ms(&g, TICK_MS);
    }

    gfx_close(&g);
    return 0;
}
