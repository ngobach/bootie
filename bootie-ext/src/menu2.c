#include <bootie.h>
#include <bootie-gfx.h>
#include <bootie-icons.h>
#include <bootie-img.h>
#include <bootie-ds.h>
#include <bootie-ini.h>
#include <bootie-gui.h>
#include <stdint.h>

#define CANVAS_W 820
#define CANVAS_H 680

/* Up/Down arrow scancodes (returned as scancode<<8 when ascii==0) */
#define KEY_UP   0x4800
#define KEY_DOWN 0x5000

int gmain(int argc, char *argv[], int flags) {
    (void)argc; (void)argv; (void)flags;
    #if defined(__i386__)
        {
            char find_buf[4096];
            uint32_t f0, f0h, f1, f1h;
            int find_len = bt_eval("find", find_buf, sizeof(find_buf));
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

    int counter = 0;

    while (1) {
        gfx_sprite_clear(&back, 10, 10, 25, 255);

        /* 100 random dark-grey texts */
        unsigned int s = (unsigned int)counter * 1664525u + 1013904223u;
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
        sprintf(buf, "%d", counter);
        int font_size = 96;
        int tw = gfx_text_width(buf, font_size);
        int cx = (cw - tw) / 2;
        int cy = (ch - font_size) / 2;
        gfx_sprite_draw_str(&back, &g, cx, cy, buf, 255, 220, 80, 255, font_size);
        gfx_sprite_draw_str(&back, &g, 20, ch - 40,
                            "UP/DOWN: change   ESC/Q: quit",
                            180, 180, 180, 255, 16);

        gfx_sprite_blit(&screen, &back, pad_x, pad_y);

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
