#ifndef BOOTIE_GUI_H
#define BOOTIE_GUI_H

#include <bootie-gfx.h>
#include <bootie-ds.h>

/* ------------------------------------------------------------------ */
/*  Icon Hash Map                                                     */
/* ------------------------------------------------------------------ */

typedef struct {
    char *key;
    struct gfx_sprite value;
} bt_gui_icon_entry_t;

static int bt_gui_icon_load(bt_gui_icon_entry_t **icons,
                             const char *name,
                             const unsigned char *png_data,
                             int png_size) {
    struct gfx_sprite sprite;
    int result = gfx_png_decode(png_data, png_size, &sprite);
    if (result != 0) return result;
    shput(*icons, name, sprite);
    return 0;
}

static void bt_gui_icons_destroy(bt_gui_icon_entry_t **icons) {
    int n = arrlenu(*icons);
    for (int i = 0; i < n; i++)
        gfx_sprite_destroy(&(*icons)[i].value);
    shfree(*icons);
    *icons = NULL;
}

/* ------------------------------------------------------------------ */
/*  Canvas Helper                                                     */
/* ------------------------------------------------------------------ */

static inline void bt_gui_canvas(uint32_t screen_w, uint32_t screen_h,
                                  int design_w, int design_h,
                                  int *out_cw, int *out_ch,
                                  int *out_pad_x, int *out_pad_y) {
    *out_cw = screen_w < (uint32_t)design_w ? (int)screen_w : design_w;
    *out_ch = screen_h < (uint32_t)design_h ? (int)screen_h : design_h;
    *out_pad_x = ((int)screen_w - *out_cw) / 2;
    *out_pad_y = ((int)screen_h - *out_ch) / 2;
}

/* ------------------------------------------------------------------ */
/*  Window Decorations                                                */
/* ------------------------------------------------------------------ */

#define BT_GUI_BORDER_R 80
#define BT_GUI_BORDER_G 80
#define BT_GUI_BORDER_B 120
#define BT_GUI_BORDER_A 255

static inline void bt_gui_border(struct gfx_sprite *s,
                                  int x, int y, int w, int h) {
    gfx_sprite_fill(s, x, y, w, 1, BT_GUI_BORDER_R, BT_GUI_BORDER_G, BT_GUI_BORDER_B, BT_GUI_BORDER_A);
    gfx_sprite_fill(s, x, y + h - 1, w, 1, BT_GUI_BORDER_R, BT_GUI_BORDER_G, BT_GUI_BORDER_B, BT_GUI_BORDER_A);
    gfx_sprite_fill(s, x, y, 1, h, BT_GUI_BORDER_R, BT_GUI_BORDER_G, BT_GUI_BORDER_B, BT_GUI_BORDER_A);
    gfx_sprite_fill(s, x + w - 1, y, 1, h, BT_GUI_BORDER_R, BT_GUI_BORDER_G, BT_GUI_BORDER_B, BT_GUI_BORDER_A);
}

static inline void bt_gui_sep(struct gfx_sprite *s, int x, int y, int w) {
    gfx_sprite_fill(s, x, y, w, 1, BT_GUI_BORDER_R, BT_GUI_BORDER_G, BT_GUI_BORDER_B, BT_GUI_BORDER_A);
}

/* ------------------------------------------------------------------ */
/*  Overlay Functions                                                 */
/* ------------------------------------------------------------------ */

static inline void bt_gui_overlay_flip(struct gfx_sprite *screen,
                                        struct gfx_sprite *back,
                                        struct gfx *ctx,
                                        int pad_x, int pad_y) {
    gfx_sprite_blit(screen, back, pad_x, pad_y);
}

static int bt_gui_confirm(struct gfx_sprite *screen, struct gfx_sprite *s,
                           struct gfx *ctx, int cw, int ch,
                           int pad_x, int pad_y,
                           const char *prompt, const char *detail) {
    gfx_sprite_fill(s, 0, 0, cw, ch, 0, 0, 0, 180);

    int dw = 400;
    int has_detail = detail && detail[0];
    int dh = has_detail ? 120 : 80;
    int dx = (cw - dw) / 2;
    int dy = (ch - dh) / 2;

    gfx_sprite_fill(s, dx, dy, dw, dh, 25, 25, 55, 255);
    bt_gui_border(s, dx, dy, dw, dh);

    gfx_sprite_draw_str(s, ctx, dx + 16, dy + 16, prompt,
                        255, 255, 200, 255, 20);
    if (has_detail)
        gfx_sprite_draw_str(s, ctx, dx + 16, dy + 52, detail,
                            180, 180, 220, 255, 16);
    gfx_sprite_draw_str(s, ctx, dx + 16, dy + dh - 28,
                        "[Enter] Confirm   [Esc] Cancel",
                        180, 180, 200, 255, 16);

    bt_gui_overlay_flip(screen, s, ctx, pad_x, pad_y);

    while (1) {
        int key = gfx_getkey(ctx);
        int ascii = key & 0xFF;
        if (ascii == 0x0D) return 1;
        if (ascii == 0x1B) return 0;
    }
}

static void bt_gui_show_info(struct gfx_sprite *screen, struct gfx_sprite *s,
                               struct gfx *ctx, int cw, int ch,
                               int pad_x, int pad_y, const char *msg) {
    gfx_sprite_fill(s, 0, 0, cw, ch, 0, 0, 0, 180);

    int dw = 400;
    int dh = 100;
    int dx = (cw - dw) / 2;
    int dy = (ch - dh) / 2;

    gfx_sprite_fill(s, dx, dy, dw, dh, 25, 25, 55, 255);
    bt_gui_border(s, dx, dy, dw, dh);

    gfx_sprite_draw_str(s, ctx, dx + 16, dy + 24, msg,
                        200, 200, 200, 255, 28);
    gfx_sprite_draw_str(s, ctx, dx + 16, dy + dh - 28,
                        "Press any key to continue",
                        150, 150, 180, 255, 16);

    bt_gui_overlay_flip(screen, s, ctx, pad_x, pad_y);
    gfx_getkey(ctx);
}

static void bt_gui_boot_feedback(struct gfx_sprite *s, struct gfx *ctx,
                                  int cw, int ch, int footer_h,
                                  const char *status, const char *extra) {
    gfx_sprite_fill(s, 0, ch - footer_h * 2, cw, footer_h * 2,
                    60, 20, 20, 255);
    gfx_sprite_draw_str(s, ctx, 8, ch - footer_h * 2 + 2,
                        status, 255, 50, 50, 255, 16);
    if (extra)
        gfx_sprite_draw_str(s, ctx, 8, ch - footer_h + 2,
                            extra, 200, 200, 200, 255, 16);
}

#endif /* BOOTIE_GUI_H */
