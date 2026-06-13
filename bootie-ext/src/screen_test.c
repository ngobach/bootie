/*
 * screen_test.c - framebuffer graphics test suite
 *
 * Cycles through test modes with SPACE, exits with ESC.
 * FPS counter in the top-right corner.
 */
#include <bootie.h>
#include <stdint.h>
#include <bootie-gfx.h>

/* ------------------------------------------------------------------ */
/*  Test modes                                                          */
/* ------------------------------------------------------------------ */
enum {
    TEST_STATIC,
    TEST_RECT_FILL,
    TEST_TEXT_FILL,
    TEST_COUNT
};

static const char *test_names[TEST_COUNT] = {
    "Static Demo",
    "Rectangle Fill Bench",
    "Text Fill Bench"
};

/* Per-test frame delay in ms (static needs low rate, benches run hot) */
static const int test_delay[TEST_COUNT] = { 50, 5, 5 };

/* ------------------------------------------------------------------ */
/*  RDTSC timer                                                         */
/* ------------------------------------------------------------------ */
static inline uint64_t read_tsc(void) {
#if defined(__i386__)
    uint64_t v;
    __asm__ volatile("rdtsc" : "=A"(v));
    return v;
#else
    uint32_t lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
#endif
}

/* ------------------------------------------------------------------ */
/*  FPS helpers                                                         */
/* ------------------------------------------------------------------ */
static uint64_t tsc_freq;
static uint64_t fps_last_tsc;
static int fps_counter;
static int fps_display;

static void fps_init(struct gfx *g) {
    uint64_t t0 = read_tsc();
    gfx_delay_ms(g, 100);
    tsc_freq = (read_tsc() - t0) * 10;
    fps_last_tsc = read_tsc();
    fps_counter = 0;
    fps_display = 0;
}

static void fps_tick(void) {
    fps_counter++;
    uint64_t now = read_tsc();
    if (now - fps_last_tsc >= tsc_freq) {
        fps_display = fps_counter;
        fps_counter = 0;
        fps_last_tsc = now;
    }
}

static void fps_draw(struct gfx *g) {
    int px = 14;
    char buf[32];
    sprintf(buf, "%d FPS", fps_display);
    int tw = gfx_text_width(buf, px);
    int x = (int)g->width - tw - 6;
    int y = 2;
    int pw = tw + 12;
    int ph = px + 10;
    fill_rect(g, x, y, pw, ph, 10, 12, 22);
    draw_str(g, x + 2, y + 3, buf, 80, 220, 80, px);
}

/* ------------------------------------------------------------------ */
/*  Test 1: static demo                                                 */
/* ------------------------------------------------------------------ */
static void draw_static(struct gfx *g) {
    uint32_t W = g->width, H = g->height;

    fill_rect(g, 0, 0, W, H, 10, 14, 26);

    uint8_t bar_colors[7][3] = {
        {255,  50,  50}, {255, 160,  30}, {255, 220,  30},
        { 50, 210,  50}, { 30, 140, 255}, { 80,  50, 220},
        {180,  50, 220},
    };
    uint32_t bar_h = H / 3 / 7;
    for (int i = 0; i < 7; i++) {
        uint32_t by = H * 2 / 3 + i * bar_h;
        fill_rect(g, 0, by, W, bar_h,
                  bar_colors[i][0], bar_colors[i][1], bar_colors[i][2]);
    }

    uint32_t grad_y = H * 2 / 3 - 4;
    for (uint32_t x = 0; x < W; x++) {
        uint8_t t = (uint8_t)((x * 255) / (W > 1 ? W - 1 : 1));
        for (uint32_t y = grad_y; y < grad_y + 4; y++)
            put_pixel(g, x, y, t, 80, 255 - t);
    }

    const char *title = "Bootie-ext GFX Test";
    uint32_t px_size = 28;
    uint32_t tx = (W > gfx_text_width(title, px_size)) ?
                  (W - gfx_text_width(title, px_size)) / 2 : 0;
    uint32_t ty = H / 4;
    draw_str(g, tx + 2, ty + 2, title, 10, 10, 40, px_size);
    draw_str(g, tx, ty, title, 220, 240, 255, px_size);

    const char *sub = "SPACE to switch, ESC to exit";
    uint32_t sx2 = (W > gfx_text_width(sub, 28)) ?
                   (W - gfx_text_width(sub, 28)) / 2 : 0;
    draw_str(g, sx2, ty + px_size + 8, sub, 160, 180, 200, 28);

    draw_strf(g, 8, 8, 120, 200, 100, 16, "Mode: %dx%d bpp=%d",
              (int)W, (int)H, (int)(g->bpp * 8));

    uint32_t bw = 3;
    fill_rect(g, 0, 0, W, bw, 80, 120, 200);
    fill_rect(g, 0, H - bw, W, bw, 80, 120, 200);
    fill_rect(g, 0, 0, bw, H, 80, 120, 200);
    fill_rect(g, W - bw, 0, bw, H, 80, 120, 200);
}

static void test_static(struct gfx *g, int first_frame) {
    (void)first_frame;
    draw_static(g);
}

/* ------------------------------------------------------------------ */
/*  Test 2: rectangle fill bench                                        */
/* ------------------------------------------------------------------ */
static void test_rect_fill(struct gfx *g, int first_frame) {
    uint32_t W = g->width, H = g->height;
    (void)first_frame;

    int rw = (int)(next_rand() % (W / 2)) + 4;
    int rh = (int)(next_rand() % (H / 2)) + 4;
    int rx = (int)(next_rand() % (W - (uint32_t)rw + 1));
    int ry = (int)(next_rand() % (H - (uint32_t)rh + 1));
    fill_rect(g, (uint32_t)rx, (uint32_t)ry, (uint32_t)rw, (uint32_t)rh,
              (uint8_t)next_rand(), (uint8_t)next_rand(), (uint8_t)next_rand());
}

/* ------------------------------------------------------------------ */
/*  Test 3: text fill bench                                             */
/* ------------------------------------------------------------------ */
static void test_text_fill(struct gfx *g, int first_frame) {
    uint32_t W = g->width, H = g->height;
    (void)first_frame;

    int px = (int)(next_rand() % 3) * 4 + 12;
    int x = (int)(next_rand() % (W - 60));
    int y = (int)(next_rand() % (H - (uint32_t)px - 4));

    char buf[32];
    sprintf(buf, "Hello #%d", (int)(next_rand() % 1000));
    draw_str(g, x, y, buf,
             (uint8_t)next_rand(), (uint8_t)next_rand(), (uint8_t)next_rand(), px);
}

/* ------------------------------------------------------------------ */
/*  Common UI overlays                                                  */
/* ------------------------------------------------------------------ */
static void draw_overlays(struct gfx *g, int test) {
    fps_draw(g);

    draw_str(g, 8, (int)g->height - 20, test_names[test], 160, 160, 160, 14);

    const char *hint = "SPACE: Next   ESC: Exit";
    int px = 14;
    int tw = gfx_text_width(hint, px);
    int x = ((int)g->width - tw) / 2;
    draw_str(g, x + 1, (int)g->height - 20 + 1, hint, 0, 0, 0, px);
    draw_str(g, x, (int)g->height - 20, hint, 200, 200, 200, px);
}

/* ------------------------------------------------------------------ */
/*  Entry point                                                         */
/* ------------------------------------------------------------------ */
int gmain(int argc, char *argv[], int flags) {
    (void)argc; (void)argv; (void)flags;
    struct gfx g;

    if (!gfx_init(&g))
        return 0;
    gfx_font_load();
    fps_init(&g);

    int test = 0;
    int running = 1;
    int first_frame = 1;
    int clear_on_next = 0;

    while (running) {
        gfx_backbuffer_begin(&g);

        if (clear_on_next) {
            fill_rect(&g, 0, 0, g.width, g.height, 0, 0, 0);
            clear_on_next = 0;
        }

        switch (test) {
        case TEST_STATIC:    test_static(&g, first_frame);    break;
        case TEST_RECT_FILL: test_rect_fill(&g, first_frame); break;
        case TEST_TEXT_FILL: test_text_fill(&g, first_frame); break;
        }

        if (first_frame)
            first_frame = 0;

        draw_overlays(&g, test);
        fps_tick();
        gfx_backbuffer_end(&g);

        gfx_delay_ms(&g, test_delay[test]);

        while (gfx_checkkey(&g)) {
            int key = gfx_getkey(&g);
            if (key == ' ') {
                test = (test + 1) % TEST_COUNT;
                first_frame = 1;
                clear_on_next = 1;
            } else if (key == 27) {
                running = 0;
            }
        }
    }

    gfx_close(&g);
    return 0;
}
