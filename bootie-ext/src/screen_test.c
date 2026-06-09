/*
 * screen_test.c - VBE graphics test (pure C, no LVGL)
 *
 * Tests VBE mode detection, linear framebuffer pixel writes,
 * and a basic font renderer using a built-in 5x7 bitmap font.
 */
#include <bootie.h>
#include <stdint.h>
#include <bootie-gfx.h>

/* ------------------------------------------------------------------ */
/*  Main graphics demo                                                  */
/* ------------------------------------------------------------------ */
static void draw_demo(struct gfx *g) {
    uint32_t W = gfx_width(g), H = gfx_height(g);

    /* --- dark background --- */
    fill_rect(g, 0, 0, W, H, 10, 14, 26);

    /* --- horizontal colour bars (rainbow) in the lower third --- */
    uint8_t bar_colors[7][3] = {
        {255,  50,  50}, /* red    */
        {255, 160,  30}, /* orange */
        {255, 220,  30}, /* yellow */
        { 50, 210,  50}, /* green  */
        { 30, 140, 255}, /* blue   */
        { 80,  50, 220}, /* indigo */
        {180,  50, 220}, /* violet */
    };
    uint32_t bar_h = H / 3 / 7;
    for (int i = 0; i < 7; i++) {
        uint32_t by = H * 2 / 3 + i * bar_h;
        fill_rect(g, 0, by, W, bar_h,
                  bar_colors[i][0], bar_colors[i][1], bar_colors[i][2]);
    }

    /* --- horizontal gradient strip just above the bars --- */
    uint32_t grad_y = H * 2 / 3 - 4;
    for (uint32_t x = 0; x < W; x++) {
        uint8_t t = (uint8_t)((x * 255) / (W > 1 ? W - 1 : 1));
        for (uint32_t y = grad_y; y < grad_y + 4; y++)
            put_pixel(g, x, y, t, 80, 255 - t);
    }

    /* --- centred title banner --- */
    const char *title = "Bootie-ext VBE Test";
    uint32_t scale   = 3;
    uint32_t tx = (W > gfx_text_width(title, SCALE_PX(scale))) ?
                  (W - gfx_text_width(title, SCALE_PX(scale))) / 2 : 0;
    uint32_t ty = H / 4;
    /* shadow */
    draw_str(g, tx + 2, ty + 2, title, 10, 10, 40, scale);
    /* main text */
    draw_str(g, tx, ty, title, 220, 240, 255, scale);

    /* --- sub-title --- */
    const char *sub = "Press any key to exit";
    uint32_t sx2 = (W > gfx_text_width(sub, SCALE_PX(2))) ?
                   (W - gfx_text_width(sub, SCALE_PX(2))) / 2 : 0;
    draw_str(g, sx2, ty + SCALE_PX(scale) + 8, sub, 160, 180, 200, 2);

    /* --- resolution info line --- */
    draw_strf(g, 8, 8, 120, 200, 100, 1, "Mode: %dx%d bpp=%d",
              (int)W, (int)H, (int)(g->bpp * 8));

    /* --- simple border --- */
    uint32_t bw = 3;
    fill_rect(g, 0, 0, W, bw, 80, 120, 200);
    fill_rect(g, 0, H-bw, W, bw, 80, 120, 200);
    fill_rect(g, 0, 0, bw, H, 80, 120, 200);
    fill_rect(g, W-bw, 0, bw, H, 80, 120, 200);
}

/* ------------------------------------------------------------------ */
/*  Entry point                                                         */
/* ------------------------------------------------------------------ */
int gmain(int argc, char *argv[], int flags) {
    (void)argc;
    (void)argv;
    (void)flags;
    struct gfx g;

    if (!gfx_init(&g)) {
        return 0;
    }
    gfx_font_load();

    /* --- draw the demo --- */
    draw_demo(&g);

    printf("Demo drawn. Press any key to return...\n");

    /* wait for keypress */
    while (!gfx_checkkey(&g)) {
        gfx_delay_ms(&g, 55);
    }
    gfx_getkey(&g);

    /* --- restore text mode --- */
    gfx_close(&g);

    return 0;
}
