#include <bootie.h>
#include <bootie-gfx.h>
#include <bootie-icons.h>
#include <bootie-img.h>
#include <bootie-ds.h>
#include <stdint.h>

#define PATH_MAX 260
#define LINE_H   48
#define HEADER_H 72
#define FOOTER_H 20
#define CANVAS_W 820
#define CANVAS_H 680

enum action_type {
    ACTION_NONE,
    ACTION_DISK_IMAGE,
    ACTION_FILE_BROWSER,
    ACTION_CHAINLOAD,
    ACTION_REBOOT,
    ACTION_POWEROFF,
};

struct menu_action {
    int type;
    char target[PATH_MAX];
};

struct menu_item {
    char title[64];
    char desc[128];
    int icon_id;
    struct menu_action action;
};

struct menu {
    struct menu_item *items;
    int cur;
    int top;
    int view_rows;
    struct gfx_sprite icons[3];
};

static void ensure_visible(struct menu *m) {
    int count = arrlenu(m->items);
    if (count == 0) {
        m->top = 0;
        m->cur = 0;
        return;
    }
    if (m->cur < 0) m->cur = 0;
    if (m->cur >= (int)count) m->cur = (int)count - 1;

    if (m->cur < m->top)
        m->top = m->cur;
    if (m->cur >= m->top + m->view_rows)
        m->top = m->cur - m->view_rows + 1;
}

static void draw(struct menu *m, struct gfx_sprite *s, struct gfx *ctx,
                  int cw, int ch) {
    gfx_sprite_fill(s, 0, 0, s->w, s->h, 15, 15, 30, 255);
    gfx_sprite_fill(s, 0, 0, cw, ch, 15, 15, 30, 255);

    gfx_sprite_fill(s, 0, 0, cw, 1, 80, 80, 120, 255);
    gfx_sprite_fill(s, 0, ch - 1, cw, 1, 80, 80, 120, 255);
    gfx_sprite_fill(s, 0, 0, 1, ch, 80, 80, 120, 255);
    gfx_sprite_fill(s, cw - 1, 0, 1, ch, 80, 80, 120, 255);

    gfx_sprite_draw_str(s, ctx, 8, 12, "Boot Menu", 200, 200, 255, 255, 2);

    gfx_sprite_fill(s, 0, 70, cw, 1, 80, 80, 120, 255);

    int x = 8;
    int y = HEADER_H;
    int start = m->top;
    int count = arrlenu(m->items);
    int end = start + m->view_rows;
    if (end > count) end = count;

    if (count == 0) {
        gfx_sprite_draw_str(s, ctx, x, y, "(no menu items)",
                            150, 150, 180, 255, 1);
    }

    for (int i = start; i < end; i++) {
        struct menu_item *item = &m->items[i];
        int row_y = y + (i - start) * LINE_H;

        if (i == m->cur) {
            gfx_sprite_fill(s, 2, row_y - 2, cw - 4, LINE_H, 40, 60, 120, 255);
            gfx_sprite_fill(s, 2, row_y - 2, cw - 4, LINE_H, 60, 80, 160, 100);
        }

        int icon_y = row_y + (LINE_H - 16) / 2;
        if (item->icon_id >= 0 && item->icon_id < 3
            && m->icons[item->icon_id].pixels) {
            gfx_sprite_blit(s, &m->icons[item->icon_id], x, icon_y);
        }

        int tx = x + 24;
        int tcolor = (i == m->cur) ? 255 : 200;
        gfx_sprite_draw_str(s, ctx, tx, row_y + 6, item->title,
                            tcolor, tcolor, 255, 255, 1);

        if (item->desc[0]) {
            gfx_sprite_draw_str(s, ctx, tx + 4, row_y + 26, item->desc,
                                160, 160, 190, 255, 1);
        }
    }

    int footer_y = ch - FOOTER_H;
    gfx_sprite_fill(s, 0, footer_y, cw, FOOTER_H, 30, 30, 60, 255);
    gfx_sprite_draw_str(s, ctx, 8, footer_y + 2,
                        "[^v] Nav  [Enter] Select  [Esc] Quit",
                        150, 150, 180, 255, 1);
}

static void overlay_flip(struct gfx_sprite *screen, struct gfx_sprite *back,
                          struct gfx *ctx, int cw, int ch) {
    int pad_x = ((int)ctx->width - cw) / 2;
    int pad_y = ((int)ctx->height - ch) / 2;
    gfx_sprite_blit(screen, back, pad_x, pad_y);
}

static int confirm_action(struct gfx_sprite *screen, struct gfx_sprite *s,
                           struct gfx *ctx, int cw, int ch,
                           const char *prompt, const char *detail) {
    gfx_sprite_fill(s, 0, 0, cw, ch, 0, 0, 0, 180);

    int dw = 400;
    int dh = 120;
    int dx = (cw - dw) / 2;
    int dy = (ch - dh) / 2;

    gfx_sprite_fill(s, dx, dy, dw, dh, 25, 25, 55, 255);
    gfx_sprite_fill(s, dx, dy, dw, 1, 120, 120, 160, 255);
    gfx_sprite_fill(s, dx, dy + dh - 1, dw, 1, 120, 120, 160, 255);
    gfx_sprite_fill(s, dx, dy, 1, dh, 120, 120, 160, 255);
    gfx_sprite_fill(s, dx + dw - 1, dy, 1, dh, 120, 120, 160, 255);

    gfx_sprite_draw_str(s, ctx, dx + 16, dy + 16, prompt, 255, 255, 200, 255, 2);
    if (detail && detail[0]) {
        gfx_sprite_draw_str(s, ctx, dx + 16, dy + 44, detail,
                            180, 180, 220, 255, 1);
    }
    gfx_sprite_draw_str(s, ctx, dx + 16, dy + dh - 32,
                        "[Y] Yes   [N] No   [ESC] Cancel",
                        180, 180, 200, 255, 1);

    overlay_flip(screen, s, ctx, cw, ch);

    while (1) {
        int key = gfx_getkey(ctx);
        int ascii = key & 0xFF;
        if (ascii == 'y' || ascii == 'Y') return 1;
        if (ascii == 'n' || ascii == 'N') return 0;
        if (ascii == 0x1B) return 0;
    }
}

static void overlay_info(struct gfx_sprite *screen, struct gfx_sprite *s,
                          struct gfx *ctx, int cw, int ch, const char *msg) {
    gfx_sprite_fill(s, 0, 0, cw, ch, 0, 0, 0, 180);

    int dw = 400;
    int dh = 100;
    int dx = (cw - dw) / 2;
    int dy = (ch - dh) / 2;

    gfx_sprite_fill(s, dx, dy, dw, dh, 25, 25, 55, 255);
    gfx_sprite_fill(s, dx, dy, dw, 1, 120, 120, 160, 255);
    gfx_sprite_fill(s, dx, dy + dh - 1, dw, 1, 120, 120, 160, 255);
    gfx_sprite_fill(s, dx, dy, 1, dh, 120, 120, 160, 255);
    gfx_sprite_fill(s, dx + dw - 1, dy, 1, dh, 120, 120, 160, 255);

    gfx_sprite_draw_str(s, ctx, dx + 16, dy + 24, msg, 200, 200, 200, 255, 2);
    gfx_sprite_draw_str(s, ctx, dx + 16, dy + dh - 28,
                        "Press any key to continue", 150, 150, 180, 255, 1);

    overlay_flip(screen, s, ctx, cw, ch);
    gfx_getkey(ctx);
}

static void handle_disk_image(struct gfx_sprite *screen, struct gfx_sprite *s,
                               struct gfx *ctx, int cw, int ch,
                               const char *target) {
    if (!confirm_action(screen, s, ctx, cw, ch, "Boot Disk Image?", target))
        return;

    char cmd[PATH_MAX + 128];
    int tlen = strlen(target);
    if (tlen >= 4 && strnicmp(target + tlen - 4, ".efi", 4) == 0) {
        sprintf(cmd, "chainloader %s ;; boot", target);
    } else {
        sprintf(cmd, "map %s (0xff) ;; map --hook ;; chainloader (0xff) ;; boot",
                target);
    }
    run_line(cmd, BUILTIN_CMDLINE);

    overlay_flip(screen, s, ctx, cw, ch);
    gfx_sprite_fill(s, 0, ch - FOOTER_H * 2, cw, FOOTER_H * 2, 60, 20, 20, 255);
    gfx_sprite_draw_str(s, ctx, 8, ch - FOOTER_H * 2 + 2,
                        "Boot failed", 255, 50, 50, 255, 1);
    gfx_getkey(ctx);
}

static void handle_chainload(struct gfx_sprite *screen, struct gfx_sprite *s,
                              struct gfx *ctx, int cw, int ch,
                              const char *target) {
    if (!confirm_action(screen, s, ctx, cw, ch, "Chainload Bootloader?", target))
        return;

    char cmd[PATH_MAX + 128];
    sprintf(cmd, "chainloader %s ;; boot", target);
    run_line(cmd, BUILTIN_CMDLINE);

    overlay_flip(screen, s, ctx, cw, ch);
    gfx_sprite_fill(s, 0, ch - FOOTER_H * 2, cw, FOOTER_H * 2, 60, 20, 20, 255);
    gfx_sprite_draw_str(s, ctx, 8, ch - FOOTER_H * 2 + 2,
                        "Boot failed", 255, 50, 50, 255, 1);
    gfx_getkey(ctx);
}

static void handle_reboot(struct gfx_sprite *screen, struct gfx_sprite *s,
                           struct gfx *ctx, int cw, int ch) {
    if (confirm_action(screen, s, ctx, cw, ch, "Restart system?", NULL))
        run_line("reboot", BUILTIN_CMDLINE);
}

static void handle_poweroff(struct gfx_sprite *screen, struct gfx_sprite *s,
                             struct gfx *ctx, int cw, int ch) {
    if (confirm_action(screen, s, ctx, cw, ch, "Shut down system?", NULL))
        run_line("halt", BUILTIN_CMDLINE);
}

static void build_demo_menu(struct menu *m) {
    struct menu_item *item;

    item = arraddnptr(m->items, 1);
    memset(item, 0, sizeof(*item));
    strcpy(item->title, "Boot Disk Image");
    strcpy(item->desc, "/boot/disk.img");
    item->icon_id = 0;
    item->action.type = ACTION_DISK_IMAGE;
    strcpy(item->action.target, "/boot/disk.img");

    item = arraddnptr(m->items, 1);
    memset(item, 0, sizeof(*item));
    strcpy(item->title, "File Browser");
    strcpy(item->desc, "Browse and boot files from disk");
    item->icon_id = 1;
    item->action.type = ACTION_FILE_BROWSER;

    item = arraddnptr(m->items, 1);
    memset(item, 0, sizeof(*item));
    strcpy(item->title, "Chainload Bootloader");
    strcpy(item->desc, "/bootmgr");
    item->icon_id = 2;
    item->action.type = ACTION_CHAINLOAD;
    strcpy(item->action.target, "/bootmgr");

    item = arraddnptr(m->items, 1);
    memset(item, 0, sizeof(*item));
    strcpy(item->title, "Restart");
    strcpy(item->desc, "Reboot the system");
    item->icon_id = 2;
    item->action.type = ACTION_REBOOT;

    item = arraddnptr(m->items, 1);
    memset(item, 0, sizeof(*item));
    strcpy(item->title, "Shutdown");
    strcpy(item->desc, "Power off the system");
    item->icon_id = 2;
    item->action.type = ACTION_POWEROFF;
}

int gmain(int argc, char *argv[], int flags) {
    (void)argc;
    (void)argv;
    (void)flags;

    struct gfx g;
    if (!gfx_init(&g))
        return 1;
    gfx_font_load();

    uint32_t fw = g.width;
    uint32_t fh = g.height;
    int cw = fw < CANVAS_W ? (int)fw : CANVAS_W;
    int ch = fh < CANVAS_H ? (int)fh : CANVAS_H;
    int pad_x = ((int)fw - cw) / 2;
    int pad_y = ((int)fh - ch) / 2;

    struct menu *m = malloc(sizeof(struct menu));
    if (!m) {
        gfx_close(&g);
        return 1;
    }
    memset(m, 0, sizeof(struct menu));
    m->view_rows = (ch - HEADER_H - FOOTER_H) / LINE_H;

    gfx_png_decode(ICON_DISC_PNG, sizeof(ICON_DISC_PNG), &m->icons[0]);
    gfx_png_decode(ICON_FOLDER_PNG, sizeof(ICON_FOLDER_PNG), &m->icons[1]);
    gfx_png_decode(ICON_BOOT_PNG, sizeof(ICON_BOOT_PNG), &m->icons[2]);

    build_demo_menu(m);

    struct gfx_sprite screen = gfx_sprite_from_fb(&g);
    struct gfx_sprite back;
    gfx_sprite_init(&back, cw, ch);

    while (1) {
        gfx_sprite_clear(&back, 15, 15, 30, 255);
        draw(m, &back, &g, cw, ch);
        gfx_sprite_blit(&screen, &back, pad_x, pad_y);

        int key = gfx_getkey(&g);
        int scan = (key >> 8) & 0xFF;
        int ascii = key & 0xFF;

        if (ascii == 0x1B || ascii == 'q' || ascii == 'Q') {
            break;
        } else if (ascii == 0x0D) {
            int count = arrlenu(m->items);
            if (m->cur >= 0 && m->cur < count) {
                struct menu_item *item = &m->items[m->cur];
                switch (item->action.type) {
                case ACTION_DISK_IMAGE:
                    handle_disk_image(&screen, &back, &g, cw, ch,
                                      item->action.target);
                    break;
                case ACTION_FILE_BROWSER:
                    overlay_info(&screen, &back, &g, cw, ch,
                                 "File Browser (coming soon)");
                    break;
                case ACTION_CHAINLOAD:
                    handle_chainload(&screen, &back, &g, cw, ch,
                                     item->action.target);
                    break;
                case ACTION_REBOOT:
                    handle_reboot(&screen, &back, &g, cw, ch);
                    break;
                case ACTION_POWEROFF:
                    handle_poweroff(&screen, &back, &g, cw, ch);
                    break;
                default:
                    break;
                }
            }
        } else if (scan == 0x48) {
            m->cur--;
            ensure_visible(m);
        } else if (scan == 0x50) {
            m->cur++;
            ensure_visible(m);
        } else if (scan == 0x47) {
            m->cur = 0;
            ensure_visible(m);
        } else if (scan == 0x4F) {
            m->cur = (int)arrlenu(m->items) - 1;
            ensure_visible(m);
        } else if (scan == 0x49) {
            m->cur -= m->view_rows;
            ensure_visible(m);
        } else if (scan == 0x51) {
            m->cur += m->view_rows;
            ensure_visible(m);
        }
    }

    gfx_sprite_destroy(&back);
    for (int i = 0; i < 3; i++)
        gfx_sprite_destroy(&m->icons[i]);
    arrfree(m->items);
    free(m);
    gfx_close(&g);
    return 0;
}
