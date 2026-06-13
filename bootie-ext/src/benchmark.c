/*
 * benchmark.c - bare-metal performance profiler
 *
 * Measures CPU, memory, and graphics throughput via RDTSC timing.
 * Results are displayed on-screen and written to isa-debugcon.
 *
 * Build: auto-discovered by $(wildcard src/*.c).
 * Run:   from menu.ini as type=program, target=benchmark
 */

#include <bootie.h>
#include <bootie-io.h>
#include <bootie-utils.h>
#include <bootie-gfx.h>
#include <bootie-gui.h>
#include <bootie-icons.h>
#include <bootie-img.h>
#include <bootie-anim.h>

/* ------------------------------------------------------------------ */
/*  Timer: RDTSC                                                       */
/* ------------------------------------------------------------------ */
static inline uint64_t rdtsc(void) {
    uint32_t lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

/* ------------------------------------------------------------------ */
/*  Scratch buffer for memcpy tests                                    */
/* ------------------------------------------------------------------ */
#define SCRATCH_SIZE (1024 * 1024)   /* 1 MB */
static unsigned char *scratch_src;
static unsigned char *scratch_dst;

/* ------------------------------------------------------------------ */
/*  Auto-calibrating benchmark runner                                  */
/* ------------------------------------------------------------------ */
struct bench_result {
    const char *name;
    int iters;
    int tsc;
};

static struct bench_result run_bench(const char *name,
                                      void (*body)(void *ctx, uint64_t iter),
                                      void *ctx, uint64_t batch,
                                      uint64_t target_tsc) {
    body(ctx, batch);

    uint64_t t0 = rdtsc();
    body(ctx, batch);
    uint64_t t1 = rdtsc();
    uint64_t batch_tsc = t1 - t0;
    if (batch_tsc == 0) batch_tsc = 1;

    uint64_t total_iters = batch;
    while (total_iters < batch * 10000) {
        uint64_t test = total_iters + batch;
        if (test < total_iters) break;
        uint32_t test_hi = (uint32_t)(test >> 32);
        uint32_t batch_tsc_lo = (uint32_t)batch_tsc;
        uint32_t target_lo = (uint32_t)target_tsc;
        uint32_t batch_lo = (uint32_t)batch;
        uint64_t lhs = (test & 0xFFFFFFFF) * (uint64_t)batch_tsc_lo;
        uint64_t rhs = (uint64_t)target_lo * (uint64_t)batch_lo;
        if (test_hi > 0 || lhs >= rhs) break;
        total_iters = test;
    }

    t0 = rdtsc();
    body(ctx, total_iters);
    t1 = rdtsc();
    uint64_t elapsed = t1 - t0;
    if (elapsed == 0) elapsed = 1;

    struct bench_result r;
    r.name = name;
    r.iters = (int)total_iters;
    r.tsc = (int)elapsed;
    return r;
}

/* ------------------------------------------------------------------ */
/*  Benchmark 1: integer arithmetic                                    */
/* ------------------------------------------------------------------ */
struct ctx_arith { volatile int x; };
static void bench_arith(void *ctx, uint64_t iter) {
    struct ctx_arith *c = (struct ctx_arith *)ctx;
    int x = c->x;
    for (uint64_t i = 0; i < iter; i++) {
        x = (x * 7 + 3) / 2 - (i & 0xFF);
        x = (x << 3) | (x >> 5);
        x ^= (int)i;
    }
    c->x = x;
}

/* ------------------------------------------------------------------ */
/*  Benchmark 2: PRNG (next_rand)                                     */
/* ------------------------------------------------------------------ */
struct ctx_prng { volatile int dummy; };
static void bench_prng(void *ctx, uint64_t iter) {
    (void)ctx;
    for (uint64_t i = 0; i < iter; i++)
        next_rand();
}

/* ------------------------------------------------------------------ */
/*  Benchmark 3: memcpy bandwidth                                     */
/* ------------------------------------------------------------------ */
struct ctx_memcpy { unsigned char *src; unsigned char *dst; int size; };
static void bench_memcpy(void *ctx, uint64_t iter) {
    struct ctx_memcpy *c = (struct ctx_memcpy *)ctx;
    for (uint64_t i = 0; i < iter; i++)
        memmove(c->dst, c->src, (unsigned int)c->size);
}

/* ------------------------------------------------------------------ */
/*  Benchmark 4: sprite fill (memset)                                 */
/* ------------------------------------------------------------------ */
struct ctx_fill { struct gfx_sprite *s; int w, h; };
static void bench_fill(void *ctx, uint64_t iter) {
    struct ctx_fill *c = (struct ctx_fill *)ctx;
    for (uint64_t i = 0; i < iter; i++)
        gfx_sprite_fill(c->s, 0, 0, c->w, c->h,
                        (uint8_t)(i & 0xFF), 80, 160, 255);
}

/* ------------------------------------------------------------------ */
/*  Benchmark 5: sprite alpha-blit (small)                             */
/* ------------------------------------------------------------------ */
struct ctx_blit { struct gfx_sprite *dst; struct gfx_sprite *src; };
static void bench_blit(void *ctx, uint64_t iter) {
    struct ctx_blit *c = (struct ctx_blit *)ctx;
    for (uint64_t i = 0; i < iter; i++)
        gfx_sprite_blit(c->dst, c->src, 0, 0);
}

/* ------------------------------------------------------------------ */
/*  Benchmark 6: sprite alpha-blit (fullscreen)                        */
/* ------------------------------------------------------------------ */
struct ctx_blit_fs { struct gfx_sprite *dst; struct gfx_sprite *src; };
static void bench_blit_fs(void *ctx, uint64_t iter) {
    struct ctx_blit_fs *c = (struct ctx_blit_fs *)ctx;
    for (uint64_t i = 0; i < iter; i++)
        gfx_sprite_blit(c->dst, c->src, 0, 0);
}

/* ------------------------------------------------------------------ */
/*  Benchmark 6: font rendering                                       */
/* ------------------------------------------------------------------ */
struct ctx_font {
    struct gfx_sprite *s; struct gfx *g;
    const char *text; int px;
};
static void bench_font(void *ctx, uint64_t iter) {
    struct ctx_font *c = (struct ctx_font *)ctx;
    for (uint64_t i = 0; i < iter; i++)
        gfx_sprite_draw_str(c->s, c->g, 4, 4, c->text,
                            200, 200, 220, 255, c->px);
}

/* ------------------------------------------------------------------ */
/*  Benchmark 7: PNG decode                                           */
/* ------------------------------------------------------------------ */
struct ctx_png { const unsigned char *data; int size; };
static void bench_png(void *ctx, uint64_t iter) {
    struct ctx_png *c = (struct ctx_png *)ctx;
    for (uint64_t i = 0; i < iter; i++) {
        struct gfx_sprite out;
        gfx_png_decode(c->data, (unsigned int)c->size, &out);
        if (out.pixels) gfx_sprite_destroy(&out);
    }
}

/* ------------------------------------------------------------------ */
/*  Benchmark 8: easing math                                          */
/* ------------------------------------------------------------------ */
struct ctx_ease { volatile float acc; };
static void bench_ease(void *ctx, uint64_t iter) {
    struct ctx_ease *c = (struct ctx_ease *)ctx;
    float acc = c->acc;
    for (uint64_t i = 0; i < iter; i++) {
        float t = (float)(i & 0x7FFF) / 32767.0f;
        acc += bt_ease_out_cubic(t);
        acc += bt_ease_out_bounce(t);
        acc += bt_ease_in_out_quad(t);
    }
    c->acc = acc;
}

/* ------------------------------------------------------------------ */
/*  Entry point                                                        */
/* ------------------------------------------------------------------ */
int gmain(int argc, char *argv[], int flags) {
    (void)argc;
    (void)argv;
    (void)flags;

    struct gfx g;
    if (!gfx_init(&g))
        return 1;
    gfx_font_load();

    int W = (int)gfx_width(&g);
    int H = (int)gfx_height(&g);

    int cw, ch, pad_x, pad_y;
    bt_gui_canvas(W, H, 820, 680, &cw, &ch, &pad_x, &pad_y);

    struct gfx_sprite screen = gfx_sprite_from_fb(&g);
    struct gfx_sprite back;
    gfx_sprite_init(&back, cw, ch);

    scratch_src = (unsigned char *)malloc(SCRATCH_SIZE);
    scratch_dst = (unsigned char *)malloc(SCRATCH_SIZE);
    if (!scratch_src || !scratch_dst) {
        gfx_close(&g);
        return 1;
    }
    for (int i = 0; i < SCRATCH_SIZE; i++)
        scratch_src[i] = (unsigned char)(i & 0xFF);

    struct gfx_sprite icon;
    gfx_png_decode(ICON_DISC_24_PNG, sizeof(ICON_DISC_24_PNG), &icon);

    uint64_t target_tsc = 300000000;

    struct gfx_sprite test_sprite;
    gfx_sprite_init(&test_sprite, 256, 64);

    /* fullscreen blit sprites */
    int fs_w = cw;
    int fs_h = ch;
    if (fs_w > 600) fs_w = 600;
    if (fs_h > 400) fs_h = 400;
    struct gfx_sprite blit_fs_src;
    struct gfx_sprite blit_fs_dst;
    gfx_sprite_init(&blit_fs_src, fs_w, fs_h);
    gfx_sprite_init(&blit_fs_dst, fs_w, fs_h);
    for (int y = 0; y < fs_h; y++) {
        uint32_t *p = (uint32_t *)(blit_fs_src.pixels + (unsigned)y * fs_w * 4);
        for (int x = 0; x < fs_w; x++) {
            uint8_t a = (uint8_t)((x + y) & 0xFF);
            if (a < 200) a = 200;
            p[x] = (uint32_t)((x * 255 / fs_w)) | ((uint32_t)(y * 255 / fs_h) << 8) | ((uint32_t)80 << 16) | ((uint32_t)a << 24);
        }
    }

    struct bench_result results[16];
    int nres = 0;

    results[nres++] = run_bench("arith",   bench_arith,  &(struct ctx_arith){12345}, 100000, target_tsc);
    results[nres++] = run_bench("prng",    bench_prng,   &(struct ctx_prng){0},      100000, target_tsc);
    results[nres++] = run_bench("memcpy4k", bench_memcpy, &(struct ctx_memcpy){scratch_src, scratch_dst, 4096}, 1000, target_tsc);
    results[nres++] = run_bench("memcpy64k",bench_memcpy, &(struct ctx_memcpy){scratch_src, scratch_dst, 65536}, 100, target_tsc);
    results[nres++] = run_bench("memcpy1m", bench_memcpy, &(struct ctx_memcpy){scratch_src, scratch_dst, 1048576}, 10, target_tsc);
    results[nres++] = run_bench("fill256x64",bench_fill,  &(struct ctx_fill){&test_sprite, 256, 64}, 1000, target_tsc);
    results[nres++] = run_bench("blit24x24",bench_blit,  &(struct ctx_blit){&test_sprite, &icon}, 1000, target_tsc);
    results[nres++] = run_bench("blit_fs",  bench_blit_fs, &(struct ctx_blit_fs){&blit_fs_dst, &blit_fs_src}, 100, target_tsc);
    results[nres++] = run_bench("font14px", bench_font,   &(struct ctx_font){&test_sprite, &g, "Hello 123!", 14}, 100, target_tsc);
    results[nres++] = run_bench("font20px", bench_font,   &(struct ctx_font){&test_sprite, &g, "The quick brown fox jumps over the lazy dog 1234567890", 20}, 50, target_tsc);
    results[nres++] = run_bench("png",      bench_png,    &(struct ctx_png){ICON_DISC_24_PNG, (int)sizeof(ICON_DISC_24_PNG)}, 100, target_tsc);
    results[nres++] = run_bench("ease",     bench_ease,   &(struct ctx_ease){0}, 10000, target_tsc);

    gfx_sprite_clear(&back, 15, 15, 30, 255);
    int yy = 20;

    gfx_sprite_draw_str(&back, &g, 20, yy,
                        "=== BENCHMARK (iters in ~300ms, higher=better) ===",
                        220, 220, 255, 255, 18);
    yy += 30;

    char line[128];
    for (int i = 0; i < nres; i++) {
        sprintf(line, "%s %d iters", results[i].name, results[i].iters);
        gfx_sprite_draw_str(&back, &g, 24, yy, line,
                            200, 200, 220, 255, 15);
        yy += 22;
        if (yy > ch - 40) break;
    }

    yy += 12;
    gfx_sprite_draw_str(&back, &g, 24, yy,
                        "[Esc] exit",
                        140, 140, 180, 255, 14);

    gfx_sprite_blit(&screen, &back, pad_x, pad_y);

    while (1) {
        int key = gfx_getkey(&g);
        if ((key & 0xFF) == 0x1B) break;
    }

    if (icon.pixels) gfx_sprite_destroy(&icon);
    gfx_sprite_destroy(&blit_fs_dst);
    gfx_sprite_destroy(&blit_fs_src);
    gfx_sprite_destroy(&test_sprite);
    free(scratch_src);
    free(scratch_dst);
    gfx_sprite_destroy(&back);
    gfx_close(&g);
    return 0;
}
