#include <bootie.h>
#include <bootie-gfx.h>
#include <bootie-icons.h>
#include <bootie-img.h>
#include <bootie-ds.h>
#include <bootie-ini.h>
#include <bootie-gui.h>
#include <stdint.h>

#define PATH_MAX 260
#define LINE_H   48
#define HEADER_H 44
#define FOOTER_H BT_GUI_FOOTER_H
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
    char icon_name[24];
    struct menu_action action;
};

struct menu {
    struct menu_item *items;
    int cur;
    int top;
    int view_rows;
    bt_gui_icon_entry_t *icons;
    int confirm_exit;
};

static void ensure_visible(struct menu *m) {
    int count = arrlenu(m->items);
    if (count == 0) {
        m->top = 0;
        m->cur = 0;
        return;
    }
    if (m->cur < 0) {
        m->cur = (int)count - 1;
    } else if (m->cur >= (int)count) {
        m->cur = 0;
    }

    if (m->cur < m->top)
        m->top = m->cur;
    if (m->cur >= m->top + m->view_rows)
        m->top = m->cur - m->view_rows + 1;
}

static void draw(struct menu *m, struct gfx_sprite *s, struct gfx *ctx,
                  int cw, int ch) {
    char footer_count[16];
    int total = arrlenu(m->items);
    sprintf(footer_count, "%d/%d", m->cur + 1, total);

    bt_gui_rect content;
    bt_gui_window(s, ctx, cw, ch,
                  "Boot Menu", NULL,
                  "[^v] Nav  [Enter] Select  [Esc] Quit",
                  footer_count, &content);

    int x = 8;
    int y = content.y;
    int start = m->top;
    int end = start + m->view_rows;
    if (end > total) end = total;

    if (total == 0) {
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

        int icon_y = row_y + 8;
        bt_gui_icon_entry_t *entry = shgetp_null(m->icons, item->icon_name);
        if (entry)
            gfx_sprite_blit(s, &entry->value, x, icon_y);

        int tx = x + 32;
        int tcolor = (i == m->cur) ? 255 : 200;
        gfx_sprite_draw_str(s, ctx, tx, row_y + 6, item->title,
                            tcolor, tcolor, 255, 255, 16);

        if (item->desc[0]) {
            gfx_sprite_draw_str(s, ctx, tx, row_y + 26, item->desc,
                                160, 160, 190, 255, 14);
        }
    }
}

static void handle_disk_image(struct gfx_sprite *screen, struct gfx_sprite *s,
                                struct gfx *ctx, int cw, int ch,
                                int pad_x, int pad_y,
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

    bt_gui_overlay_flip(screen, s, ctx, pad_x, pad_y);
    bt_gui_boot_feedback(s, ctx, cw, ch, FOOTER_H, "Boot failed", NULL);
    gfx_getkey(ctx);
}

static void handle_chainload(struct gfx_sprite *screen, struct gfx_sprite *s,
                               struct gfx *ctx, int cw, int ch,
                               int pad_x, int pad_y,
                               const char *target) {
    char cmd[PATH_MAX + 128];
    sprintf(cmd, "chainloader %s ;; boot", target);
    run_line(cmd, BUILTIN_CMDLINE);

    bt_gui_overlay_flip(screen, s, ctx, pad_x, pad_y);
    bt_gui_boot_feedback(s, ctx, cw, ch, FOOTER_H, "Boot failed", NULL);
    gfx_getkey(ctx);
}

static void handle_reboot(struct gfx_sprite *screen, struct gfx_sprite *s,
                            struct gfx *ctx, int cw, int ch,
                            int pad_x, int pad_y) {
    if (bt_gui_confirm(screen, s, ctx, cw, ch, pad_x, pad_y,
                       "Restart system?", NULL))
        run_line("reboot", BUILTIN_CMDLINE);
}

static void handle_poweroff(struct gfx_sprite *screen, struct gfx_sprite *s,
                              struct gfx *ctx, int cw, int ch,
                              int pad_x, int pad_y) {
    if (bt_gui_confirm(screen, s, ctx, cw, ch, pad_x, pad_y,
                       "Shut down system?", NULL))
        run_line("halt", BUILTIN_CMDLINE);
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

        item->action.type = type;

        char type_icon[24] = "";
        switch (type) {
        case ACTION_DISK_IMAGE:   strcpy(type_icon, "disc");     break;
        case ACTION_FILE_BROWSER: strcpy(type_icon, "folder");   break;
        case ACTION_CHAINLOAD:    strcpy(type_icon, "boot");     break;
        case ACTION_REBOOT:       strcpy(type_icon, "restart");  break;
        case ACTION_POWEROFF:     strcpy(type_icon, "poweroff"); break;
        }

        const char *custom_icon = bt_ini_section_get_value(&ini.sections[i], "icon");
        if (custom_icon && shgetp_null(m->icons, custom_icon)) {
            strcpy(item->icon_name, custom_icon);
        } else {
            strcpy(item->icon_name, type_icon);
        }

        const char *target = bt_ini_section_get_value(&ini.sections[i], "target");
        if (target)
            strcpy(item->action.target, target);


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

    int cw, ch, pad_x, pad_y;
    bt_gui_canvas(g.width, g.height, CANVAS_W, CANVAS_H, &cw, &ch, &pad_x, &pad_y);

    struct menu *m = malloc(sizeof(struct menu));
    if (!m) {
        gfx_close(&g);
        return 1;
    }
    memset(m, 0, sizeof(struct menu));
    m->view_rows = (ch - HEADER_H - FOOTER_H) / LINE_H;

    bt_gui_icon_load(&m->icons, "disc", ICON_DISC_24_PNG);
    bt_gui_icon_load(&m->icons, "folder", ICON_FOLDER_24_PNG);
    bt_gui_icon_load(&m->icons, "boot", ICON_BOOT_24_PNG);
    bt_gui_icon_load(&m->icons, "restart", ICON_RESTART_24_PNG);
    bt_gui_icon_load(&m->icons, "poweroff", ICON_POWEROFF_24_PNG);
    bt_gui_icon_load(&m->icons, "windows", ICON_WINDOWS_24_PNG);

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
            if (!m->confirm_exit || bt_gui_confirm(&screen, &back, &g, cw, ch, pad_x, pad_y, "Quit Boot Menu?", NULL))
                break;
        } else if (ascii == 0x0D) {
            int count = arrlenu(m->items);
            if (m->cur >= 0 && m->cur < count) {
                struct menu_item *item = &m->items[m->cur];
                switch (item->action.type) {
                case ACTION_DISK_IMAGE:
                    handle_disk_image(&screen, &back, &g, cw, ch, pad_x, pad_y,
                                      item->action.target);
                    break;
                case ACTION_FILE_BROWSER:
                    {
                        char cmd[512];
                        sprintf(cmd, "%%moddir%%/file_browser %s",
                                item->action.target);
                        run_line(cmd, BUILTIN_CMDLINE);
                    }
                    break;
                case ACTION_CHAINLOAD:
                    handle_chainload(&screen, &back, &g, cw, ch, pad_x, pad_y,
                                     item->action.target);
                    break;
                case ACTION_REBOOT:
                    handle_reboot(&screen, &back, &g, cw, ch, pad_x, pad_y);
                    break;
                case ACTION_POWEROFF:
                    handle_poweroff(&screen, &back, &g, cw, ch, pad_x, pad_y);
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
    bt_gui_icons_destroy(&m->icons);
    arrfree(m->items);
    free(m);
    gfx_close(&g);
    return 0;
}
