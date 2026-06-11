#include <bootie.h>
#include <bootie-gfx.h>
#include <bootie-icons.h>
#include <bootie-img.h>
#include <bootie-ds.h>
#include <bootie-ini.h>
#include <bootie-gui.h>
#include <bootie-debug.h>
#include <stdint.h>

#define CANVAS_W 820
#define CANVAS_H 680

/* Up/Down arrow scancodes (returned as scancode<<8 when ascii==0) */
#define KEY_UP   0x4800
#define KEY_DOWN 0x5000

#if defined(__i386__)
/* Read 64-bit TSC into a pair of uint32_t */
static inline void rdtsc64(uint32_t *lo, uint32_t *hi) {
    __asm__ __volatile__("rdtsc" : "=a"(*lo), "=d"(*hi));
}

/* Measure CPU MHz using PIT channel 0 (18.2 Hz tick at 0x46C in BDA).
 * Waits for 2 PIT ticks (~110 ms) and counts TSC cycles. */
static uint32_t measure_cpu_mhz(void) {
    volatile uint32_t *bda_tick = (volatile uint32_t *)0x46C;
    uint32_t t0 = *bda_tick;
    /* wait for tick to change */
    while (*bda_tick == t0) {}
    uint32_t lo0, hi0;
    rdtsc64(&lo0, &hi0);
    uint32_t t1 = *bda_tick;
    /* wait 2 more ticks (~110 ms) */
    while (*bda_tick < t1 + 2) {}
    uint32_t lo1, hi1;
    rdtsc64(&lo1, &hi1);
    /* cycles elapsed (ignore hi for reasonable frequencies) */
    uint32_t cycles = lo1 - lo0;
    /* 2 PIT ticks = 2/18.2065 s ≈ 0.10993 s  →  MHz = cycles / 109930 */
    return cycles / 109930;
}
#endif

static void itoa_simple(int val, char *buf, int buflen) {
    if (buflen < 2) return;
    int neg = 0;
    if (val < 0) { neg = 1; val = -val; }
    char tmp[16];
    int i = 0;
    if (val == 0) { tmp[i++] = '0'; }
    while (val > 0 && i < 15) { tmp[i++] = '0' + (val % 10); val /= 10; }
    if (neg && i < 15) tmp[i++] = '-';
    int out = 0;
    for (int j = i - 1; j >= 0 && out < buflen - 1; j--)
        buf[out++] = tmp[j];
    buf[out] = '\0';
}

int gmain(int argc, char *argv[], int flags) {
    (void)argc; (void)argv; (void)flags;

#if defined(__i386__)
    {
        char find_buf[4096];
        uint32_t f0, f0h, f1, f1h;
        rdtsc64(&f0, &f0h);
        int find_len = bt_eval("find", find_buf, sizeof(find_buf));
        rdtsc64(&f1, &f1h);
        bt_dbg_printf("pre_find len=%d cyc=%u (Kcyc)\n",
            (long)find_len,
            (unsigned int)((f1 - f0) / 1000));
    }
#endif

    /* ---- Dump CPU memory state BEFORE gfx_init (output to debugcon) ---- */
#if defined(__i386__)
    {
        uint32_t cr0, cr3;
        __asm__("mov %%cr0, %0" : "=r"(cr0));
        __asm__("mov %%cr3, %0" : "=r"(cr3));
        uint32_t pat_lo, pat_hi;
        __asm__("rdmsr" : "=a"(pat_lo), "=d"(pat_hi) : "c"(0x277));
        uint32_t def_lo, def_hi, cap_lo, cap_hi;
        __asm__("rdmsr" : "=a"(def_lo), "=d"(def_hi) : "c"(0x2FF));
        __asm__("rdmsr" : "=a"(cap_lo), "=d"(cap_hi) : "c"(0xFE));

        bt_dbg_printf("=== CPU state ===\n");
        bt_dbg_printf("CR0=0x%x paging=%d\n",
            (unsigned long)cr0, (int)((cr0 >> 31) & 1));
        bt_dbg_printf("CR3=0x%x\n", (unsigned long)cr3);
        bt_dbg_printf("IA32_PAT hi=0x%x lo=0x%x\n",
            (unsigned long)pat_hi, (unsigned long)pat_lo);
        bt_dbg_printf("MTRR_DEF_TYPE=0x%x\n",
            (unsigned long)(def_lo & 0xFF));

        /* x87 FPU control word — precision control (PC) at bits 9:8
         *   00=single  01=rsvd  10=double  11=extended(default)
         * QEMU TCG on ARM has no native 80-bit FPU: extended precision
         * is emulated in software and can be ~6x slower than double. */
        uint16_t fpu_cw = 0x037F; /* default fallback */
        __asm__ __volatile__("fstcw %0" : "=m"(fpu_cw));
        bt_dbg_printf("FPU CW=0x%x  PC=%d (0=single 2=double 3=extended)\n",
            (unsigned long)fpu_cw, (int)((fpu_cw >> 8) & 3));

        int vcnt = (int)(cap_lo & 0xFF);
        if (vcnt > 8) vcnt = 8;
        for (int i = 0; i < vcnt; i++) {
            uint32_t b_lo, b_hi, m_lo, m_hi;
            __asm__("rdmsr" : "=a"(b_lo), "=d"(b_hi) : "c"(0x200 + 2*i));
            __asm__("rdmsr" : "=a"(m_lo), "=d"(m_hi) : "c"(0x201 + 2*i));
            if (m_lo & 0x800)
                bt_dbg_printf(" MTRR[%d] base=0x%x type=%u mask=0x%x\n",
                    (long)i,
                    (unsigned long)(b_lo & ~0xFFFu),
                    (unsigned long)(b_lo & 0xFF),
                    (unsigned long)(m_lo & ~0xFFFu));
        }
        bt_dbg_printf("=== end ===\n");
    }
#endif


    struct gfx g;
    if (!gfx_init(&g)) return 1;


    gfx_font_load();

    int cw, ch, pad_x, pad_y;
    bt_gui_canvas(g.width, g.height, CANVAS_W, CANVAS_H, &cw, &ch, &pad_x, &pad_y);

    struct gfx_sprite screen = gfx_sprite_from_fb(&g);
    struct gfx_sprite back;
    gfx_sprite_init(&back, cw, ch);

    /* Diagnostics: print where malloc placed the backbuffer + CPU MHz */
#if defined(__i386__)
    printf("back.pixels addr : 0x%08X\n", (unsigned int)(uintptr_t)back.pixels);
    printf("back size        : %u bytes\n", (unsigned int)(cw * ch * 4));
    printf("framebuffer addr : 0x%08X\n", (unsigned int)(uintptr_t)g.fb);
    printf("Measuring CPU MHz (wait ~110ms)...\n");
    uint32_t cpu_mhz = measure_cpu_mhz();
    printf("CPU speed        : ~%u MHz\n", (unsigned int)cpu_mhz);
    printf("Press any key to start...\n");
    bios_getkey();
#endif

    int counter = 0;
    unsigned int seed = 12345;
    uint32_t frame_cycles = 0;

    /* ---- JIT warm-up: render 5 frames without blocking on input ----
     * In QEMU TCG mode, sft_render's x86 code is JIT-compiled into host
     * "translation blocks" on first execution.  These frames force that
     * compilation before the interactive loop so that BIOS-call mode
     * switches (gfx_getkey) cannot evict freshly-compiled TBs.
     * We log each warmup frame so we can see if cost drops frame-by-frame. */
    bt_dbg_printf("=== warmup frames ===\n");
    for (int wf = 0; wf < 5; wf++) {
#if defined(__i386__)
        uint32_t w0, w0h, w1, w1h, w2, w2h, w3, w3h;
        rdtsc64(&w0, &w0h);
#endif
        gfx_sprite_clear(&back, 10, 10, 25, 255);
#if defined(__i386__)
        rdtsc64(&w1, &w1h);
#endif
        unsigned int ws = seed;
        static const char wn[] = "The quick brown fox jumps over the lazy dog 0123456789";
        for (int i = 0; i < 100; i++) {
            ws = ws * 1664525u + 1013904223u; int rx = (int)(ws % (unsigned int)cw);
            ws = ws * 1664525u + 1013904223u; int ry = (int)(ws % (unsigned int)ch);
            ws = ws * 1664525u + 1013904223u; int rl = 8 + (int)(ws % 20);
            char tmp[32]; for (int j = 0; j < rl && j < 31; j++) tmp[j] = wn[(ws+(unsigned int)j)%(sizeof(wn)-1)]; tmp[rl]='\0';
            gfx_sprite_draw_str(&back, &g, rx, ry, tmp, 50, 50, 55, 255, 14);
        }
#if defined(__i386__)
        rdtsc64(&w2, &w2h);
#endif
        gfx_sprite_blit(&screen, &back, pad_x, pad_y);
#if defined(__i386__)
        rdtsc64(&w3, &w3h);
        bt_dbg_printf("warmup[%d] clr=%u txt=%u blt=%u (Kcyc)\n",
            wf,
            (unsigned int)((w1-w0)/1000),
            (unsigned int)((w2-w1)/1000),
            (unsigned int)((w3-w2)/1000));
#endif
    }
    bt_dbg_printf("=== end warmup ===\n");


    while (1) {
        /* --- render with per-step timing --- */
        uint32_t t0, t0h, t1, t1h, t2, t2h, t3, t3h;

#if defined(__i386__)
        rdtsc64(&t0, &t0h);
#endif
        gfx_sprite_clear(&back, 10, 10, 25, 255);

        /* 100 random dark-grey texts */
#if defined(__i386__)
        rdtsc64(&t1, &t1h);
#endif
        unsigned int s = seed;
        static const char noise_str[] = "The quick brown fox jumps over the lazy dog 0123456789";
        for (int i = 0; i < 100; i++) {
            s = s * 1664525u + 1013904223u;
            int rx = (int)(s % (unsigned int)cw);
            s = s * 1664525u + 1013904223u;
            int ry = (int)(s % (unsigned int)ch);
            s = s * 1664525u + 1013904223u;
            int rlen = 8 + (int)(s % 20);
            char tmp[32];
            for (int j = 0; j < rlen && j < 31; j++)
                tmp[j] = noise_str[(s + (unsigned int)j) % (sizeof(noise_str) - 1)];
            tmp[rlen] = '\0';
            gfx_sprite_draw_str(&back, &g, rx, ry, tmp, 50, 50, 55, 255, 14);
        }

        /* big counter */
        char buf[24];
        itoa_simple(counter, buf, sizeof(buf));
        int font_size = 96;
        int tw = gfx_text_width(buf, font_size);
        int cx = (cw - tw) / 2;
        int cy = (ch - font_size) / 2;
        gfx_sprite_draw_str(&back, &g, cx, cy, buf, 255, 220, 80, 255, font_size);
        gfx_sprite_draw_str(&back, &g, 20, ch - 40,
                            "UP/DOWN: change   ESC/Q: quit",
                            180, 180, 180, 255, 16);

#if defined(__i386__)
        rdtsc64(&t2, &t2h);
#endif
        /* perf overlay (uses previous frame's numbers) */
#if defined(__i386__)
        char perf[80];
        sprintf(perf, "clr:%uM txt:%uM blt:%uM",
                (unsigned int)((t1-t0)/1000000),
                (unsigned int)((t2-t1)/1000000),
                (unsigned int)(frame_cycles/1000000));
        gfx_sprite_draw_str(&back, &g, 20, ch - 64, perf, 80, 200, 80, 255, 14);
#endif

        gfx_sprite_blit(&screen, &back, pad_x, pad_y);

#if defined(__i386__)
        rdtsc64(&t3, &t3h);
        frame_cycles = t3 - t0;
        /* log every frame to debugcon */
        bt_dbg_printf("clr=%u txt=%u blt=%u total=%u (Kcyc)\n",
            (unsigned int)((t1-t0)/1000),
            (unsigned int)((t2-t1)/1000),
            (unsigned int)((t3-t2)/1000),
            (unsigned int)(frame_cycles/1000));
#endif

        int key = gfx_getkey(&g);
        int ascii = key & 0xFF;
        if (ascii == 0x1B || ascii == 'q' || ascii == 'Q') break;
        if (key == KEY_UP)   counter++;
        if (key == KEY_DOWN) counter--;
    }



    gfx_sprite_destroy(&back);
    gfx_close(&g);
    return 0;
}
