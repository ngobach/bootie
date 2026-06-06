/*
 * screen_test.c - VBE graphics test (pure C, no LVGL)
 *
 * Tests VBE mode detection, linear framebuffer pixel writes,
 * and a basic font renderer using a built-in 5x7 bitmap font.
 */
#include <bootprog.h>
#include <stdint.h>
#include <gfx.h>

/* ------------------------------------------------------------------ */
/*  Main graphics demo                                                  */
/* ------------------------------------------------------------------ */
static void draw_demo(void) {
    uint32_t W = fb_width, H = fb_height;

    /* --- dark background --- */
    fill_rect(0, 0, W, H, 10, 14, 26);

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
        fill_rect(0, by, W, bar_h,
                  bar_colors[i][0], bar_colors[i][1], bar_colors[i][2]);
    }

    /* --- horizontal gradient strip just above the bars --- */
    uint32_t grad_y = H * 2 / 3 - 4;
    for (uint32_t x = 0; x < W; x++) {
        uint8_t t = (uint8_t)((x * 255) / (W > 1 ? W - 1 : 1));
        for (uint32_t y = grad_y; y < grad_y + 4; y++)
            put_pixel(x, y, t, 80, 255 - t);
    }

    /* --- centred title banner --- */
    const char *title = "Bootie-ext VBE Test";
    uint32_t scale   = 3;
    uint32_t char_w  = (5 + 1) * scale;
    uint32_t str_len = 0;
    for (const char *p = title; *p; p++) str_len++;
    uint32_t tx = (W > str_len * char_w) ? (W - str_len * char_w) / 2 : 0;
    uint32_t ty = H / 4;
    /* shadow */
    draw_str(tx + 2, ty + 2, title, 10, 10, 40, scale);
    /* main text */
    draw_str(tx, ty, title, 220, 240, 255, scale);

    /* --- sub-title --- */
    const char *sub = "Press any key to exit";
    uint32_t slen = 0;
    for (const char *p = sub; *p; p++) slen++;
    uint32_t sx2 = (W > slen * (5+1)*2) ? (W - slen * (5+1)*2) / 2 : 0;
    draw_str(sx2, ty + 7*scale + 8, sub, 160, 180, 200, 2);

    /* --- resolution info line --- */
    /* build a small string manually with printf into a local buffer */
    char info[64];
    int pos = 0;
    /* "Mode: WxH bpp=B" */
    info[pos++] = 'M'; info[pos++] = 'o'; info[pos++] = 'd';
    info[pos++] = 'e'; info[pos++] = ':'; info[pos++] = ' ';
    /* width digits */
    uint32_t tmp = W; int start_pos = pos;
    if (tmp == 0) { info[pos++] = '0'; } else {
        char digs[10]; int nd = 0;
        while (tmp) { digs[nd++] = '0' + (tmp % 10); tmp /= 10; }
        for (int i = nd-1; i >= 0; i--) info[pos++] = digs[i];
    }
    info[pos++] = 'x';
    tmp = H;
    if (tmp == 0) { info[pos++] = '0'; } else {
        char digs[10]; int nd = 0;
        while (tmp) { digs[nd++] = '0' + (tmp % 10); tmp /= 10; }
        for (int i = nd-1; i >= 0; i--) info[pos++] = digs[i];
    }
    info[pos++] = ' '; info[pos++] = 'b'; info[pos++] = 'p';
    info[pos++] = 'p'; info[pos++] = '=';
    tmp = fb_bpp * 8;
    if (tmp == 0) { info[pos++] = '0'; } else {
        char digs[10]; int nd = 0;
        while (tmp) { digs[nd++] = '0' + (tmp % 10); tmp /= 10; }
        for (int i = nd-1; i >= 0; i--) info[pos++] = digs[i];
    }
    info[pos] = '\0';
    (void)start_pos;

    draw_str(8, 8, info, 120, 200, 100, 1);

    /* --- simple border --- */
    uint32_t bw = 3;
    fill_rect(0, 0, W, bw, 80, 120, 200);
    fill_rect(0, H-bw, W, bw, 80, 120, 200);
    fill_rect(0, 0, bw, H, 80, 120, 200);
    fill_rect(W-bw, 0, bw, H, 80, 120, 200);
}

/* ------------------------------------------------------------------ */
/*  Entry point                                                         */
/* ------------------------------------------------------------------ */
int gmain(int argc, char *argv[], int flags) {
    printf("Bootie-ext: VBE screen test starting\n");

    /* --- find best VBE mode --- */
    struct vbe_driver_info drv;
    if (!get_driver_info(&drv)) {
        printf("VBE 2.0 not supported\n");
        return 0;
    }

    uint16_t *modes = (uint16_t *)drv.VideoModePtr;
    uint16_t best_mode  = 0;
    int      best_score = -1;
    struct vbe_mode_info best_mi;

    for (int idx = 0; modes[idx] != 0xFFFF; idx++) {
        uint16_t mode = modes[idx];
        struct vbe_mode_info mi;
        if (!get_mode_info(mode, &mi)) continue;

        /* skip non-packed-pixel memory models */
        if (mi.MemoryModel != 4 && mi.MemoryModel != 6) continue;

        int score = 0;
        if      (mi.BitsPerPixel == 32) score += 100;
        else if (mi.BitsPerPixel == 24) score +=  90;
        else if (mi.BitsPerPixel == 16) score +=  50;
        else continue;

        if      (mi.XResolution == 1024 && mi.YResolution == 768)  score += 60;
        else if (mi.XResolution ==  800 && mi.YResolution == 600)  score += 50;
        else if (mi.XResolution ==  640 && mi.YResolution == 480)  score += 30;
        else score += 5;

        if (score > best_score) {
            best_score = score;
            best_mode  = mode;
            best_mi    = mi;
        }
    }

    if (!best_mode) {
        printf("No suitable VBE mode found\n");
        return 0;
    }

    printf("Setting VBE mode 0x%X (%dx%d, %d bpp)\n",
           (int)best_mode,
           (int)best_mi.XResolution, (int)best_mi.YResolution,
           (int)best_mi.BitsPerPixel);

    if ((bios_int10(0x4F02, 0x4000 | best_mode, 0, 0, (unsigned long)-1, 0) & 0xFF) != 0x4F) {
        printf("Failed to set VBE mode\n");
        return 0;
    }

    /* --- set up framebuffer globals --- */
    fb        = (uint8_t *)best_mi.PhysBasePtr;
    fb_width  = best_mi.XResolution;
    fb_height = best_mi.YResolution;
    fb_pitch  = best_mi.LinBytesPerScanline ? best_mi.LinBytesPerScanline
                                            : best_mi.BytesPerScanline;
    fb_bpp    = (best_mi.BitsPerPixel + 7) / 8;

    /* determine channel shifts from VBE colour mask info */
    fb_rshift = best_mi.LinRedFieldPosition   ? best_mi.LinRedFieldPosition
                                              : best_mi.RedFieldPosition;
    fb_gshift = best_mi.LinGreenFieldPosition ? best_mi.LinGreenFieldPosition
                                              : best_mi.GreenFieldPosition;
    fb_bshift = best_mi.LinBlueFieldPosition  ? best_mi.LinBlueFieldPosition
                                              : best_mi.BlueFieldPosition;

    printf("fb=0x%08X pitch=%u bpp=%u rgb_shifts=(%u,%u,%u)\n",
           (unsigned int)fb, (unsigned int)fb_pitch, (unsigned int)fb_bpp,
           (unsigned int)fb_rshift, (unsigned int)fb_gshift, (unsigned int)fb_bshift);

    /* --- draw the demo --- */
    draw_demo();

    printf("Demo drawn. Press any key to return...\n");

    /* wait for keypress */
    while (!bios_checkkey()) {
        delay_ms(55);
    }
    bios_getkey();

    /* --- restore text mode --- */
    if (graphics_inited) {
        if (current_term->STARTUP)
            ((int (*)(int))current_term->STARTUP)(0);
    } else {
        bios_int10(3, 0, 0, 0, (unsigned long)-1, 0);
    }
    cls();

    return 0;
}
