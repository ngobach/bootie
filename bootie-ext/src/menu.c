#include <bootie.h>
#include <bootie-gfx.h>
#include <bootie-icons.h>
#include <bootie-img.h>
#include <bootie-ds.h>
#include <bootie-ini.h>
#include <stdint.h>

#define PATH_MAX 260
#define LINE_H   48
#define HEADER_H 44
#define FOOTER_H 28
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
    int priority;
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
    struct gfx_sprite icons[5];
    int confirm_exit;
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

    gfx_sprite_draw_str(s, ctx, 8, 12, "Boot Menu", 200, 200, 255, 255, 28);

    gfx_sprite_fill(s, 0, HEADER_H - 1, cw, 1, 80, 80, 120, 255);

    int x = 8;
    int y = HEADER_H;
    int start = m->top;
    int count = arrlenu(m->items);
    int end = start + m->view_rows;
    if (end > count) end = count;

    if (count == 0) {
        gfx_sprite_draw_str(s, ctx, x, y + 8, "(no menu items)",
                            150, 150, 180, 255, 16);
    }

    for (int i = start; i < end; i++) {
        struct menu_item *item = &m->items[i];
        int row_y = y + (i - start) * LINE_H;

        if (i == m->cur) {
            gfx_sprite_fill(s, 2, row_y - 2, cw - 4, LINE_H, 40, 60, 120, 255);
            gfx_sprite_fill(s, 2, row_y - 2, cw - 4, LINE_H, 60, 80, 160, 100);
        }

        int icon_y = row_y + 4;
        if (item->icon_id >= 0 && item->icon_id < 5
            && m->icons[item->icon_id].pixels) {
            gfx_sprite_blit(s, &m->icons[item->icon_id], x, icon_y);
        }

        int tx = x + 32;
        int tcolor = (i == m->cur) ? 255 : 200;
        gfx_sprite_draw_str(s, ctx, tx, row_y + 6, item->title,
                            tcolor, tcolor, 255, 255, 16);

        if (item->desc[0]) {
            gfx_sprite_draw_str(s, ctx, tx, row_y + 26, item->desc,
                                160, 160, 190, 255, 14);
        }
    }

    int footer_y = ch - FOOTER_H;
    gfx_sprite_fill(s, 0, footer_y, cw, FOOTER_H, 30, 30, 60, 255);
    gfx_sprite_draw_str(s, ctx, 8, footer_y + 7,
                        "[^v] Nav  [Enter] Select  [Esc] Quit",
                        150, 150, 180, 255, 16);

    char footer_count[16];
    int total = arrlenu(m->items);
    sprintf(footer_count, "%d/%d", m->cur + 1, total);
    gfx_sprite_draw_str(s, ctx, cw - 8 - (int)strlen(footer_count) * 8,
                        footer_y + 7, footer_count, 200, 200, 230, 255, 16);
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
    int has_detail = detail && detail[0];
    int dh = has_detail ? 120 : 80;
    int dx = (cw - dw) / 2;
    int dy = (ch - dh) / 2;

    gfx_sprite_fill(s, dx, dy, dw, dh, 25, 25, 55, 255);
    gfx_sprite_fill(s, dx, dy, dw, 1, 120, 120, 160, 255);
    gfx_sprite_fill(s, dx, dy + dh - 1, dw, 1, 120, 120, 160, 255);
    gfx_sprite_fill(s, dx, dy, 1, dh, 120, 120, 160, 255);
    gfx_sprite_fill(s, dx + dw - 1, dy, 1, dh, 120, 120, 160, 255);

    gfx_sprite_draw_str(s, ctx, dx + 16, dy + 16, prompt, 255, 255, 200, 255, 20);
    if (has_detail) {
        gfx_sprite_draw_str(s, ctx, dx + 16, dy + 52, detail,
                            180, 180, 220, 255, 16);
    }
    gfx_sprite_draw_str(s, ctx, dx + 16, dy + dh - 28,
                        "[Enter] Confirm   [Esc] Cancel",
                        180, 180, 200, 255, 16);

    overlay_flip(screen, s, ctx, cw, ch);

    while (1) {
        int key = gfx_getkey(ctx);
        int ascii = key & 0xFF;
        if (ascii == 0x0D) return 1;
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

    gfx_sprite_draw_str(s, ctx, dx + 16, dy + 24, msg, 200, 200, 200, 255, 28);
    gfx_sprite_draw_str(s, ctx, dx + 16, dy + dh - 28,
                        "Press any key to continue", 150, 150, 180, 255, 16);

    overlay_flip(screen, s, ctx, cw, ch);
    gfx_getkey(ctx);
}

static void handle_disk_image(struct gfx_sprite *screen, struct gfx_sprite *s,
                               struct gfx *ctx, int cw, int ch,
                               const char *target) {
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
                        "Boot failed", 255, 50, 50, 255, 16);
    gfx_getkey(ctx);
}

static void handle_chainload(struct gfx_sprite *screen, struct gfx_sprite *s,
                              struct gfx *ctx, int cw, int ch,
                              const char *target) {
    char cmd[PATH_MAX + 128];
    sprintf(cmd, "chainloader %s ;; boot", target);
    run_line(cmd, BUILTIN_CMDLINE);

    overlay_flip(screen, s, ctx, cw, ch);
    gfx_sprite_fill(s, 0, ch - FOOTER_H * 2, cw, FOOTER_H * 2, 60, 20, 20, 255);
    gfx_sprite_draw_str(s, ctx, 8, ch - FOOTER_H * 2 + 2,
                        "Boot failed", 255, 50, 50, 255, 16);
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

static int icon_id_from_name(const char *name) {
    if (!name) return -1;
    if (stricmp(name, "disc") == 0) return 0;
    if (stricmp(name, "folder") == 0) return 1;
    if (stricmp(name, "boot") == 0) return 2;
    if (stricmp(name, "restart") == 0) return 3;
    if (stricmp(name, "poweroff") == 0) return 4;
    return -1;
}

static int action_type_from_name(const char *name) {
    if (!name) return ACTION_NONE;
    if (stricmp(name, "disk-image") == 0) return ACTION_DISK_IMAGE;
    if (stricmp(name, "file-browser") == 0) return ACTION_FILE_BROWSER;
    if (stricmp(name, "chainload") == 0) return ACTION_CHAINLOAD;
    if (stricmp(name, "reboot") == 0) return ACTION_REBOOT;
    if (stricmp(name, "poweroff") == 0) return ACTION_POWEROFF;
    return ACTION_NONE;
}

static void load_ini_items(struct menu *m) {
    struct bt_ini ini;
    if (bt_ini_parse_file(&ini, "/menu.ini") != 0)
        return;

    for (int i = 0; i < ini.section_count; i++) {
        const char *sec_name = ini.sections[i].name;
        if (strnicmp(sec_name, "items.", 6) != 0)
            continue;

        const char *type_str = bt_ini_section_get_value(&ini.sections[i], "type");
        if (!type_str) continue;

        int type = action_type_from_name(type_str);
        if (type == ACTION_NONE) continue;

        const char *title = bt_ini_section_get_value(&ini.sections[i], "title");
        if (!title) continue;

        struct menu_item *item = arraddnptr(m->items, 1);
        memset(item, 0, sizeof(*item));

        strcpy(item->title, title);

        const char *desc = bt_ini_section_get_value(&ini.sections[i], "desc");
        if (desc)
            strcpy(item->desc, desc);

        const char *icon_str = bt_ini_section_get_value(&ini.sections[i], "icon");
        item->icon_id = icon_id_from_name(icon_str);

        item->action.type = type;

        const char *target = bt_ini_section_get_value(&ini.sections[i], "target");
        if (target)
            strcpy(item->action.target, target);

        item->action.priority = bt_ini_section_get_int(&ini.sections[i], "priority", 1000);
    }

    m->confirm_exit = bt_ini_get_bool(&ini, "menu", "confirm_exit", 1);
    bt_ini_destroy(&ini);
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

    gfx_png_decode(ICON_DISC_24_PNG, sizeof(ICON_DISC_24_PNG), &m->icons[0]);
    gfx_png_decode(ICON_FOLDER_16_PNG, sizeof(ICON_FOLDER_16_PNG), &m->icons[1]);
    gfx_png_decode(ICON_BOOT_16_PNG, sizeof(ICON_BOOT_16_PNG), &m->icons[2]);
    gfx_png_decode(ICON_RESTART_24_PNG, sizeof(ICON_RESTART_24_PNG), &m->icons[3]);
    gfx_png_decode(ICON_POWEROFF_24_PNG, sizeof(ICON_POWEROFF_24_PNG), &m->icons[4]);

    load_ini_items(m);

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
            if (!m->confirm_exit || confirm_action(&screen, &back, &g, cw, ch, "Quit Boot Menu?", NULL))
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
    for (int i = 0; i < 5; i++)
        gfx_sprite_destroy(&m->icons[i]);
    arrfree(m->items);
    free(m);
    gfx_close(&g);
    return 0;
}
