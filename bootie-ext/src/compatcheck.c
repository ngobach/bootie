/*
 * compatcheck.c - system compatibility diagnostic (console-based)
 *
 * Tests memory, graphics, timer, keyboard input, and text output.
 * Default output is text console so the tool works even if GFX is broken.
 * GFX/input/output tests open the framebuffer for interactive verification.
 *
 * Usage:
 *   compatcheck --mem      Memory allocator stress test
 *   compatcheck --gfx      Graphics mode + framebuffer test
 *   compatcheck --timer    Timer resolution + calibration test
 *   compatcheck --input    Keyboard input echo test
 *   compatcheck --output   Text rendering output test
 *   compatcheck --all      Run all tests (default)
 */
#include <bootie.h>
#include <stdint.h>
#include <bootie-gfx.h>
#if !defined(__i386__)
#include <uefi/gfx.h>
#endif
#include <bootie-ds.h>

/* ------------------------------------------------------------------ */
/*  Console helpers                                                    */
/* ------------------------------------------------------------------ */

static void print_tag(const char *tag, const char *detail) {
    putstr(tag);
    putstr("  ");
    putstr(detail);
    putchar('\n');
}

static void print_result(int pass, const char *tag, const char *detail) {
    putstr(pass ? "[PASS] " : "[FAIL] ");
    print_tag(tag, detail);
}

static void print_info(const char *tag, const char *detail) {
    putstr("       ");
    print_tag(tag, detail);
}

static void print_header(const char *text) {
    putstr("\n");
    putstr(text);
    putchar('\n');
}

/* ------------------------------------------------------------------ */
/*  Hex / number formatting                                            */
/* ------------------------------------------------------------------ */
static void fmt_hex32(char *buf, uint32_t v) {
    buf[0] = '0'; buf[1] = 'x';
    const char *hex = "0123456789abcdef";
    int started = 0, pos = 2;
    for (int s = 28; s >= 0; s -= 4) {
        int d = (v >> s) & 0xF;
        if (d || started || s == 0) { buf[pos++] = hex[d]; started = 1; }
    }
    buf[pos] = '\0';
}

static void fmt_u32(char *buf, uint32_t v) {
    char tmp[12];
    int i = 0;
    if (v == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    while (v) { tmp[i++] = '0' + (v % 10); v /= 10; }
    for (int j = 0; j < i; j++) buf[j] = tmp[i - 1 - j];
    buf[i] = '\0';
}

static void fmt_cat(char *buf, const char *a, const char *b) {
    int n = 0;
    while (*a) buf[n++] = *a++;
    while (*b) buf[n++] = *b++;
    buf[n] = '\0';
}

/* ------------------------------------------------------------------ */
/*  Memory test                                                        */
/* ------------------------------------------------------------------ */
static void test_mem(void) {
    print_header("=== MEMORY TEST ===");

    uint32_t ms = (uint32_t)free_mem_start;
    uint32_t me = (uint32_t)free_mem_end;
    if (ms || me) {
        char buf[64], tmp[16];
        fmt_hex32(buf, ms);
        { int n = 0; while (buf[n]) n++; buf[n++] = ' '; buf[n++] = '-'; buf[n++] = ' '; fmt_hex32(buf + n, me); }
        print_info("heap", buf);

        uint32_t diff = (me > ms) ? me - ms : 0;
        fmt_u32(tmp, diff >> 20);
        fmt_cat(buf, tmp, " MB");
        print_info("size", buf);
    }

    /* Binary search for largest successful malloc (up to 16 MB) */
    unsigned int lo = 1024, hi = 16U * 1024 * 1024, best = 0;
    putstr("  searching max malloc...\n");
    while (lo <= hi) {
        unsigned int mid = lo + (hi - lo) / 2;
        putstr(".");
        void *p = malloc(mid);
        if (p) { free(p); best = mid; lo = mid + 1; }
        else   { hi = mid - 1; }
    }
    putstr("\n");
    {
        char buf[48], tmp[16];
        fmt_u32(tmp, best);
        fmt_cat(buf, tmp, " bytes");
        print_result(best >= 256 * 1024, "max malloc", buf);
    }

    /* Alloc/free 100x random-size blocks */
    {
        int fails = 0;
        for (int i = 0; i < 100; i++) {
            unsigned int sz = 1024 + (next_rand() % 63) * 1024;
            void *p = malloc(sz);
            if (p) free(p); else fails++;
        }
        char buf[32], tmp[12];
        fmt_u32(tmp, fails);
        fmt_cat(buf, tmp, " failures");
        print_result(fails == 0, "100x random", buf);
    }

    /* 2.2 MB alloc -- same size as menu.c back sprite */
    {
        unsigned int sz = 820 * 680 * 4;
        void *p = malloc(sz);
        if (p) {
            volatile unsigned char *vp = (volatile unsigned char *)p;
            for (unsigned int i = 0; i < sz; i += 4096) vp[i] = (unsigned char)(i & 0xFF);
            free(p);
        }
        print_result(p != 0, "2.2MB alloc", p ? "back-sprite size OK" : "FAILED");
    }

    /* Realloc growth: 4KB -> 8KB, verify old data preserved */
    {
        unsigned int old_sz = 4096, new_sz = 8192;
        void *p = malloc(old_sz);
        int pass = 0;
        if (p) {
            unsigned int *vp = (unsigned int *)p;
            for (unsigned int i = 0; i < old_sz / 4; i++) vp[i] = 0xDEADBEEF;
            void *q = bt_realloc(p, new_sz);
            if (q) {
                unsigned int *vq = (unsigned int *)q;
                pass = 1;
                for (unsigned int i = 0; i < old_sz / 4; i++) {
                    if (vq[i] != 0xDEADBEEF) { pass = 0; break; }
                }
                free(q);
            }
        }
        print_result(pass, "realloc growth", pass ? "data preserved" : "FAILED");
    }

    putstr("\nPress any key to continue...\n");
    getkey();
}

/* ------------------------------------------------------------------ */
/*  Graphics test -- opens GFX to verify framebuffer write/read        */
/* ------------------------------------------------------------------ */
static void test_gfx(void) {
    print_header("=== GRAPHICS TEST ===");

    struct gfx g;
    int ok = gfx_init(&g);
    print_result(ok, "gfx init", ok ? "OK" : "FAILED");
    if (!ok) return;

    {
        char buf[48], tmp[12];
        int n = 0;
        fmt_u32(tmp, g.width); { int i = 0; while (tmp[i]) buf[n++] = tmp[i++]; }
        buf[n++] = 'x';
        fmt_u32(tmp, g.height); { int i = 0; while (tmp[i]) buf[n++] = tmp[i++]; }
        buf[n++] = ' ';
        fmt_u32(tmp, g.bpp * 8); { int i = 0; while (tmp[i]) buf[n++] = tmp[i++]; }
        buf[n++] = 'b'; buf[n++] = 'p'; buf[n++] = 'p'; buf[n] = '\0';
        print_info("mode", buf);
    }

    {
        char buf[48];
        fmt_hex32(buf, (uint32_t)(uintptr_t)g.fb);
        print_info("fb base", buf);
    }

    {
        char buf[48], tmp[16];
        uint32_t sz = g.pitch * g.height;
        fmt_u32(tmp, sz);
        fmt_cat(buf, tmp, " bytes");
        print_info("fb size", buf);
    }

    /* Color bar verification: draw 4 quadrants */
    {
        struct gfx_sprite screen;
        gfx_sprite_init(&screen, g.width, g.height);

        uint32_t hw = g.width / 2, hh = g.height / 2;
        putstr("  [1] drawing red...\n");
        gfx_sprite_fill_rect(&screen, 0,  0,  hw, hh, 255, 0,   0, 255);
        putstr("  [2] drawing green...\n");
        gfx_sprite_fill_rect(&screen, hw, 0,  hw, hh, 0,   255, 0, 255);
        putstr("  [3] drawing blue...\n");
        gfx_sprite_fill_rect(&screen, 0,  hh, hw, hh, 0,   0,   255, 255);
        putstr("  [4] drawing white...\n");
        gfx_sprite_fill_rect(&screen, hw, hh, hw, hh, 255, 255, 255, 255);

        putstr("  [5] draw_str...\n");
        draw_str(&screen, 4, 4, "GFX: color bars", 255, 255, 255, 14);

        putstr("  [6] draw done.\n");
        gfx_flush_sprite(&g, &screen);
        gfx_sprite_destroy(&screen);
        print_result(1, "fb write", "OK (readback skipped)");
    }

    putstr("  [7] about to wait for key...\n");
    putstr("  (check screen for color bars)\n");
    putstr("  Press any key to continue...\n");
#if !defined(__i386__)
    gfx_getkey(&g);
#else
    getkey();
#endif
    putstr("  [8] key received, restoring screen...\n");

    /* Restore screen */
    {
        struct gfx_sprite bg;
        gfx_sprite_init(&bg, g.width, g.height);
        gfx_sprite_clear(&bg, 15, 15, 30, 255);
        gfx_flush_sprite(&g, &bg);
        gfx_sprite_destroy(&bg);
    }
    gfx_flush(&g);
    putstr("  [9] closing gfx...\n");
    gfx_close(&g);
    putstr("  [10] done.\n");
}

/* ------------------------------------------------------------------ */
/*  Timer test                                                         */
/* ------------------------------------------------------------------ */
static void test_timer(void) {
    print_header("=== TIMER TEST ===");

    {
        char buf[32], tmp[16];
        fmt_u32(tmp, (uint32_t)tsc_khz);
        fmt_cat(buf, tmp, " kHz");
        print_result(tsc_khz >= 1000, "tsc_khz", buf);
    }

    struct gfx g;
    int ok = gfx_init(&g);
    if (ok) {
        /* Measure 3 x gfx_delay_ms(100) */
        int total_err = 0;
        for (int i = 0; i < 3; i++) {
            uint64_t t0 = bt_rdtsc();
            gfx_delay_ms(&g, 100);
            uint64_t t1 = bt_rdtsc();
#if defined(__i386__)
            uint32_t elapsed = (uint32_t)div64_32(t1 - t0, (uint32_t)tsc_khz);
#else
            uint32_t elapsed = (uint32_t)((t1 - t0) / tsc_khz);
#endif
            int err = elapsed > 100 ? elapsed - 100 : 100 - elapsed;
            total_err += err;
        }
        {
            char buf[48], tmp[12];
            fmt_u32(tmp, total_err / 3);
            fmt_cat(buf, tmp, " ms avg error");
            print_result(total_err / 3 < 20, "delay(100ms)", buf);
        }

        /* Measure millis() busy-wait */
        uint32_t t0 = (uint32_t)millis();
        uint32_t elapsed_ms;
        int ticks = 0;
        do { elapsed_ms = (uint32_t)millis() - t0; ticks++; }
        while (elapsed_ms < 100 && ticks < 1000000);

        {
            char buf[48], tmp[12], tmp2[12];
            fmt_u32(tmp, elapsed_ms);
            fmt_u32(tmp2, ticks);
            int n = 0;
            while (tmp[n]) n++;
            tmp[n++] = ' '; tmp[n++] = 'm'; tmp[n++] = 's'; tmp[n++] = ' ';
            tmp[n++] = '('; tmp[n++] = 't'; tmp[n++] = 'i'; tmp[n++] = 'c';
            tmp[n++] = 'k'; tmp[n++] = 's'; tmp[n++] = ':'; tmp[n] = '\0';
            fmt_cat(buf, tmp, tmp2);
            { int l = 0; while (buf[l]) l++; buf[l++] = ')'; buf[l] = '\0'; }
            print_result(elapsed_ms >= 90 && elapsed_ms <= 110, "millis(100ms)", buf);
        }

        gfx_close(&g);
    } else {
        putstr("  (delay tests skipped -- gfx_init failed)\n");
    }

    putstr("\nPress any key to continue...\n");
    getkey();
}

/* ------------------------------------------------------------------ */
/*  Input test -- uses GRUB4DOS getkey/checkkey (no gfx needed)        */
/* ------------------------------------------------------------------ */
static void test_input(void) {
    print_header("=== INPUT TEST ===");
    putstr("  Press any key within 2 seconds...\n");

    uint32_t start = (uint32_t)millis();
    int got_key = 0;
    int key = 0;
    while ((uint32_t)millis() - start < 2000) {
        if (checkkey()) {
            key = getkey();
            got_key = 1;
            break;
        }
        sleep(1);
    }

    if (got_key) {
        int ascii = key & 0xFF;
        int scan  = (key >> 8) & 0xFF;
        char buf[64], hex[3] = {0};
        const char *hexd = "0123456789abcdef";
        int n = 0;

        const char *pre = "key = 0x";
        while (*pre) buf[n++] = *pre++;
        hex[0] = hexd[(key >> 12) & 0xF]; hex[1] = hexd[(key >> 8) & 0xF];
        { int i = 0; while (hex[i]) buf[n++] = hex[i++]; }
        hex[0] = hexd[(key >> 4) & 0xF]; hex[1] = hexd[key & 0xF];
        { int i = 0; while (hex[i]) buf[n++] = hex[i++]; }

        const char *mid = " (asc:";
        while (*mid) buf[n++] = *mid++;
        if (ascii >= 32 && ascii < 127) {
            buf[n++] = ' '; buf[n++] = '\''; buf[n++] = ascii; buf[n++] = '\'';
        } else {
            buf[n++] = ' '; buf[n++] = '0'; buf[n++] = 'x';
            buf[n++] = hexd[(ascii >> 4) & 0xF]; buf[n++] = hexd[ascii & 0xF];
        }
        const char *mid2 = " sc:";
        while (*mid2) buf[n++] = *mid2++;
        buf[n++] = hexd[(scan >> 4) & 0xF]; buf[n++] = hexd[scan & 0xF];
        buf[n++] = ')'; buf[n] = '\0';

        print_result(1, "keyboard", buf);
    } else {
        print_result(0, "keyboard", "timeout (no input detected)");
    }

    putstr("\nPress any key to continue...\n");
    getkey();
}

/* ------------------------------------------------------------------ */
/*  Output test -- opens GFX to verify font rendering visually         */
/* ------------------------------------------------------------------ */
static void test_output(void) {
    print_header("=== OUTPUT TEST ===");

    struct gfx g;
    int ok = gfx_init(&g);
    gfx_font_load();
    if (!ok) {
        print_result(0, "gfx init", "FAILED (cannot test output)");
        return;
    }

    uint32_t W = gfx_width(&g), H = gfx_height(&g);

    struct gfx_sprite back;
    gfx_sprite_init(&back, W, H);
    gfx_sprite_clear(&back, 15, 15, 30, 255);

    /* Color gradient strip */
    for (uint32_t x = 0; x < W; x++) {
        uint8_t r = (uint8_t)((x * 255) / (W > 1 ? W - 1 : 1));
        uint8_t b = (uint8_t)(255 - r);
        gfx_sprite_fill(&back, (int)x, 0, 1, 16, r, 40, b, 255);
    }

    /* Text at 3 sizes */
    gfx_sprite_draw_str(&back, 8, 24, "OUTPUT: font 14px",
                        220, 220, 255, 255, 14);
    gfx_sprite_draw_str(&back, 8, 50, "OUTPUT: font 20px",
                        200, 255, 200, 255, 20);
    gfx_sprite_draw_str(&back, 8, 82, "OUTPUT: font 28px",
                        255, 200, 200, 255, 28);

    /* Text width measurement */
    const char *sample = "The quick brown fox";
    int tw = gfx_text_width(sample, 20);
    {
        char buf[48], tmp[12];
        int n = 0;
        const char *pre = "width('";
        while (*pre) buf[n++] = *pre++;
        while (*sample) buf[n++] = *sample++;
        buf[n++] = '\''; buf[n++] = ')'; buf[n++] = ' '; buf[n++] = '='; buf[n++] = ' ';
        fmt_u32(tmp, (uint32_t)tw);
        { int i = 0; while (tmp[i]) buf[n++] = tmp[i++]; }
        buf[n++] = 'p'; buf[n++] = 'x'; buf[n] = '\0';
        gfx_sprite_draw_str(&back, 8, 124, buf,
                            180, 180, 220, 255, 16);
    }

    /* Sample paragraph */
    gfx_sprite_draw_str(&back, 8, 156,
                        "Pack my box with five dozen liquor jugs!",
                        200, 200, 220, 255, 14);

    gfx_sprite_draw_str(&back, 8, (int)H - 24,
                        "Press any key to exit...",
                        120, 120, 160, 255, 14);
    gfx_flush_sprite(&g, &back);

    gfx_getkey(&g);

    gfx_sprite_destroy(&back);
    gfx_close(&g);

    print_result(tw > 0, "text width", tw > 0 ? "OK" : "FAILED");
    print_result(1, "font render", "OK");

    putstr("\nPress any key to continue...\n");
    getkey();
}

/* ------------------------------------------------------------------ */
/*  Entry point                                                        */
/* ------------------------------------------------------------------ */
static void show_usage(void) {
    putstr("COMPATCHECK - system compatibility diagnostic\n");
    putstr("\n");
    putstr("Usage:  compatcheck [options]\n");
    putstr("\n");
    putstr("Options:\n");
    putstr("  --mem      Memory allocator stress test\n");
    putstr("  --gfx      Graphics mode + framebuffer test\n");
    putstr("  --timer    Timer resolution + calibration test\n");
    putstr("  --input    Keyboard input echo test\n");
    putstr("  --output   Text rendering output test\n");
    putstr("  --all      Run all tests\n");
}

int gmain(int argc, char *argv[], int flags) {
    (void)flags;

    int do_mem   = 0, do_gfx = 0, do_timer = 0;
    int do_input = 0, do_output = 0, do_any = 0;

    for (int i = 1; i < argc; i++) {
        if (stricmp(argv[i], "--mem") == 0)   { do_mem = 1; do_any = 1; }
        else if (stricmp(argv[i], "--gfx") == 0)   { do_gfx = 1; do_any = 1; }
        else if (stricmp(argv[i], "--timer") == 0)  { do_timer = 1; do_any = 1; }
        else if (stricmp(argv[i], "--input") == 0)  { do_input = 1; do_any = 1; }
        else if (stricmp(argv[i], "--output") == 0) { do_output = 1; do_any = 1; }
        else if (stricmp(argv[i], "--all") == 0)    { do_mem = do_gfx = do_timer = do_input = do_output = 1; do_any = 1; }
        else { show_usage(); return 0; }
    }

    if (!do_any) { show_usage(); return 0; }

    cls();

    if (do_mem)   test_mem();
    if (do_gfx)   test_gfx();
    if (do_timer) test_timer();
    if (do_input) test_input();
    if (do_output) test_output();

    putstr("\nDone. Press any key to exit.\n");
    getkey();

    return 0;
}
