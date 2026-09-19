#include <bootie.h>
#include <bootie-io.h>
#include <bootie-utils.h>

#include <bootie-gfx.h>
#include <bootie-icons.h>
#include <bootie-img.h>
#include <bootie-ds.h>
#include <bootie-ini.h>
#include <bootie-gui.h>

#include <stdint.h>

/* Set at startup: 1 on BIOS (GRUB4DOS), 0 on UEFI (GRUB4EFI) */
static int is_bios;

#define PATH_MAX 260

/* Shared buffers to avoid large stack allocations in bare-metal/firmware.
   They are allocated once from the heap in gmain(). They must NOT be file-scope
   arrays: the loader does not reserve the module's .bss, so a large .bss (a few
   KB or more) overlaps the region the first heap allocation is taken from —
   which is the 1.92 MB screen sprite, clobbering the statics (font handle, etc.). */
#define BOOT_CMD_SIZE (PATH_MAX + 256)
#define BOOT_LOG_SIZE 10240
static char *boot_cmd;
static char *boot_log;
#define LINE_H   48
#define HEADER_H 72
#define FOOTER_H BT_GUI_FOOTER_H

enum action_type {
    ACTION_NONE,
    ACTION_DISK_IMAGE,
    ACTION_FILE_BROWSER,
    ACTION_CHAINLOAD,
    ACTION_REBOOT,
    ACTION_POWEROFF,
    ACTION_OPEN_CATEGORY,
    ACTION_PROGRAM,
    ACTION_BOOT_WIM,
};

struct menu_action {
    int type;
    char target[PATH_MAX];
};

struct menu_item {
    char title[64];
    char desc[128];
    char icon_name[24];
    char category[32];
    struct menu_action action;
};

struct cat_nav {
    char *category;
    char *display;
};

struct menu {
    struct menu_item *items;
    int cur;
    int top;
    int view_rows;
    bt_gui_icon_entry_t *icons;
    int confirm_exit;
    char  *current_category;
    char  *current_category_display;
    struct cat_nav *category_stack;
    int   *view;
};

static void ensure_visible(struct menu *m) {
    int count = arrlenu(m->view);
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
    int total = arrlenu(m->view);
    sprintf(footer_count, "%d/%d", m->cur + 1, total);

    bt_gui_rect content;
    bt_gui_window(s, ctx, cw, ch,
                  "Boot Menu", m->current_category_display,
                  "[^v] Nav  [Enter] Select  [Esc] Quit",
                  footer_count, &content);

    bt_gui_icon_entry_t *hdr_icon = shgetp_null(m->icons, "broken_robot");
    if (hdr_icon)
        gfx_sprite_draw_sprite(s, &hdr_icon->value, cw - 8 - hdr_icon->value.w,
                        (HEADER_H - hdr_icon->value.h) / 2);

    int x = 8;
    int y = content.y;
    int start = m->top;
    int end = start + m->view_rows;
    if (end > total) end = total;

    if (total == 0) {
        gfx_sprite_draw_str(s, x, y + 8, "(no menu items)",
                            150, 150, 180, 255, 16);
    }

    for (int i = start; i < end; i++) {
        struct menu_item *item = &m->items[m->view[i]];
        int row_y = y + (i - start) * LINE_H;

        if (i == m->cur) {
            gfx_sprite_fill(s, 2, row_y, cw - 4, LINE_H, 40, 60, 120, 255);
            gfx_sprite_fill(s, 2, row_y, cw - 4, LINE_H, 60, 80, 160, 100);
        }

        int icon_y = row_y + 8;
        bt_gui_icon_entry_t *entry = shgetp_null(m->icons, item->icon_name);
        if (entry)
            gfx_sprite_draw_sprite(s, &entry->value, x, icon_y);

        int tx = x + 32;
        int tcolor = (i == m->cur) ? 255 : 200;
        gfx_sprite_draw_str(s, tx, row_y + 6, item->title,
                            tcolor, tcolor, 255, 255, 16);

        if (item->desc[0]) {
            gfx_sprite_draw_str(s, tx, row_y + 26, item->desc,
                                160, 160, 190, 255, 14);
        }
    }
}

static void handle_disk_image(struct gfx *g,
                                int cw, int ch,
                                const char *target) {
    int tlen = strlen(target);
    if (tlen >= 4 &&
        (strnicmp(target + tlen - 4, ".ima", 4) == 0 ||
         strnicmp(target + tlen - 4, ".img", 4) == 0)) {
        sprintf(boot_cmd, "map --mem %s (fd0) ;; map --hook ;; root (fd0) ;; chainloader +1 ;; boot",
                target);
    } else {
        sprintf(boot_cmd, "map %s (0xff) ;; map --hook ;; root (0xff) ;; chainloader (0xff) ;; boot",
                target);
    }

    boot_log[0] = '\0';
    int bt_ret = bt_eval_ex(boot_cmd, boot_log, BOOT_LOG_SIZE, BT_EVAL_F_ERRMSG);
    if (bt_ret != 0)
        bt_gui_show_log(g, cw, ch,
                        "Boot failed", boot_log);
}

static void handle_chainload(struct gfx *g,
                               int cw, int ch,
                               const char *target) {
    sprintf(boot_cmd, "chainloader %s ;; boot", target);

    boot_log[0] = '\0';
    int bt_ret = bt_eval_ex(boot_cmd, boot_log, BOOT_LOG_SIZE, BT_EVAL_F_ERRMSG);
    if (bt_ret != 0)
        bt_gui_show_log(g, cw, ch,
                        "Boot failed", boot_log);
}

static void handle_reboot(struct gfx *g,
                            int cw, int ch) {
    if (bt_gui_confirm(g, cw, ch,
                       "Restart system?", NULL)) {
        boot_log[0] = '\0';
        int bt_ret = bt_eval_ex("reboot", boot_log, BOOT_LOG_SIZE, BT_EVAL_F_ERRMSG);
        bt_gui_show_log(g, cw, ch,
                        "Failed to reboot", boot_log);
    }
}

static void handle_poweroff(struct gfx *g,
                              int cw, int ch) {
    if (bt_gui_confirm(g, cw, ch,
                       "Shut down system?", NULL)) {
        boot_log[0] = '\0';
        int bt_ret = bt_eval_ex("halt", boot_log, BOOT_LOG_SIZE, BT_EVAL_F_ERRMSG);
        bt_gui_show_log(g, cw, ch,
                        "Failed to shut down", boot_log);
    }
}

static void handle_boot_wim(struct gfx *g,
                             int cw, int ch,
                             const char *target) {
    sprintf(boot_cmd,
            "find --set-root %s ;; uuid () ;; "
            "kernel /ntloader hires=no uuid=%%?_UUID%% wim=%s ;; "
            "initrd /initrd.cpio ;; boot",
            target, target);

    boot_log[0] = '\0';
    int bt_ret = bt_eval_ex(boot_cmd, boot_log, BOOT_LOG_SIZE, BT_EVAL_F_ERRMSG);
    if (bt_ret != 0)
        bt_gui_show_log(g, cw, ch,
                        "Boot failed", boot_log);
}

/* ------------------------------------------------------------------ */
/*  Embedded file browser                                              */
/*                                                                     */
/*  Folded in from the former standalone `file_browser` module. Items  */
/*  of type `file-browser` open it in-process. It is a sub-view of the */
/*  menu: the only way in is through a menu item, and Esc always       */
/*  returns to the menu view it was opened from (Left still goes up    */
/*  one directory, and Left at "/" falls back to the drive list).      */
/* ------------------------------------------------------------------ */

#define BR_PATH_MAX 260
#define BR_NAME_MAX 128
#define BR_MAX_DRIVES 32

#define BR_LINE_H   32
#define BR_HEADER_H 72

#define BR_NUM_BOOT_EXT 7
static const char br_bootable_ext[BR_NUM_BOOT_EXT][8] = {
    ".iso", ".img", ".ima", ".vhd", ".vhdx", ".wim", ".efi"
};

struct br_entry {
    char name[BR_NAME_MAX];
    int is_dir;
    int bootable;
    int is_drive;
    unsigned long long size;
};

struct browser {
    char cwd[BR_PATH_MAX];
    char device[24];
    struct br_entry *entries;
    int top;
    int cur;
    int view_rows;
    int show_dotfiles;
    bt_gui_icon_entry_t *icons;
};

static void br_safe_strncpy(char *dest, const char *src, int n) {
    int i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

static int br_has_boot_ext(const char *name) {
    int len = strlen(name);
    for (int i = 0; i < BR_NUM_BOOT_EXT; i++) {
        const char *ext = br_bootable_ext[i];
        int elen = strlen(ext);
        if (len >= elen && strnicmp(name + len - elen, ext, elen) == 0)
            return 1;
    }
    return 0;
}

static void br_sort_entries(struct br_entry *entries, int count) {
    for (int i = 0; i < count - 1; i++) {
        int best = i;
        for (int j = i + 1; j < count; j++) {
            if (entries[j].is_dir != entries[best].is_dir) {
                if (entries[j].is_dir)
                    best = j;
            } else {
                int cmp = 0;
                const char *sa = entries[j].name;
                const char *sb = entries[best].name;
                while (*sa && *sb && *sa == *sb) { sa++; sb++; }
                if (*sa != *sb)
                    cmp = (unsigned char)*sa - (unsigned char)*sb;
                if (cmp < 0)
                    best = j;
            }
        }
        if (best != i) {
            struct br_entry tmp = entries[i];
            entries[i] = entries[best];
            entries[best] = tmp;
        }
    }
}

/* Accepts either "(hd0,1)/dir" or a plain path, which is resolved against
   the current root device. An empty path means "show the drive list". */
static void br_parse_device_path(const char *arg, char *device, char *cwd) {
    device[0] = '\0';
    cwd[0] = '\0';
    if (arg[0] == '(') {
        const char *p = arg;
        while (*p && *p != ')') p++;
        if (*p == ')') {
            int devlen = p - arg + 1;
            if (devlen < 24) {
                for (int i = 0; i < devlen; i++) {
                    device[i] = arg[i];
                }
                device[devlen] = '\0';
                strcpy(cwd, p + 1);
                if (cwd[0] == '\0') {
                    strcpy(cwd, "/");
                }
                return;
            }
        }
    }
    if (arg[0] == '/') {
        char root[128];
        if (bt_eval("echo %@root%", root, sizeof(root)) >= 0) {
            char *trim = root;
            while (*trim == ' ' || *trim == '\t' || *trim == '\n' || *trim == '\r') trim++;
            char *e = trim + strlen(trim);
            while (e > trim && (e[-1] == ' ' || e[-1] == '\t' || e[-1] == '\n' || e[-1] == '\r')) e--;
            *e = '\0';
            br_safe_strncpy(device, trim, 24);
        }
        strcpy(cwd, arg);
        return;
    }
    strcpy(cwd, arg);
}

static int br_list_dir(struct browser *br) {
    arrfree(br->entries);
    br->entries = NULL;
    br->top = 0;
    br->cur = 0;

    if (br->cwd[0] == '\0') {
        struct bt_drive_info drives[BR_MAX_DRIVES];
        int nd = bt_device_ls(drives, BR_MAX_DRIVES);
        for (int i = 0; i < nd; i++) {
            struct br_entry e;
            strcpy(e.name, drives[i].name);
            e.is_dir = 0;
            e.bootable = 0;
            e.is_drive = 1;
            e.size = 0;
            arrput(br->entries, e);
        }
        if (arrlen(br->entries) > 1)
            br_sort_entries(br->entries, arrlen(br->entries));
        return 0;
    }

    char fullpath[BR_PATH_MAX * 2];
    strcpy(fullpath, br->device);
    int len = strlen(fullpath);
    strcpy(fullpath + len, br->cwd);

    struct bt_dir_entry *list = bt_directory_list(fullpath);
    if (!list) return -1;

    for (struct bt_dir_entry *p = list; p->name; p++) {
        if (!br->show_dotfiles && p->name[0] == '.') {
            free(p->name);
            continue;
        }

        struct br_entry e;
        strcpy(e.name, p->name);
        e.is_dir = p->is_dir;
        e.bootable = !p->is_dir && br_has_boot_ext(p->name);
        e.is_drive = 0;
        e.size = 0;
        arrput(br->entries, e);
        free(p->name);
    }
    free(list);

    {
        int count = arrlen(br->entries);
        if (count > 1)
            br_sort_entries(br->entries, count);
    }
    return 0;
}

static void br_load_selected_size(struct browser *br) {
    int count = arrlen(br->entries);
    if (br->cur < 0 || br->cur >= count) return;
    struct br_entry *e = &br->entries[br->cur];
    if (e->is_dir || e->is_drive || e->size > 0 || e->size == (unsigned long long)-1) return;

    char fullpath[BR_PATH_MAX * 2];
    strcpy(fullpath, br->device);
    int plen = strlen(fullpath);
    strcpy(fullpath + plen, br->cwd);
    plen = strlen(fullpath);
    if (plen > 0 && fullpath[plen - 1] != '/') {
        fullpath[plen++] = '/';
        fullpath[plen] = '\0';
    }
    const char *src = e->name;
    while (*src) {
        if (*src == ' ' || *src == '"' || *src == '\\')
            fullpath[plen++] = '\\';
        fullpath[plen++] = *src++;
    }
    fullpath[plen] = '\0';

    unsigned long long size = bt_file_size_at_path(fullpath);
    if (size > 0) {
        e->size = size;
    } else {
        e->size = (unsigned long long)-1;
    }
}

static void br_ensure_visible(struct browser *br) {
    int count = arrlen(br->entries);
    if (count == 0) {
        br->top = 0;
        br->cur = 0;
        return;
    }
    if (br->cur < 0) {
        br->cur = count - 1;
    } else if (br->cur >= count) {
        br->cur = 0;
    }
    if (br->top < 0) br->top = 0;
    if (br->top > br->cur) br->top = br->cur;
    if (br->cur >= br->top + br->view_rows)
        br->top = br->cur - br->view_rows + 1;
}

static void br_draw(struct browser *br, struct gfx_sprite *s, struct gfx *ctx,
                    int cw, int ch) {
    const char *title = (br->cwd[0] == '\0') ? "Select Drive" : "File Browser";

    char subtitle[BR_PATH_MAX + 24];
    const char *sub = NULL;
    if (br->cwd[0] == '\0') {
        sub = "Drives";
    } else if (br->device[0]) {
        int dlen = strlen(br->device);
        strcpy(subtitle, br->device);
        strcpy(subtitle + dlen, br->cwd);
        sub = subtitle;
    } else {
        sub = br->cwd;
    }

    const char *footer_left = (br->cwd[0] == '\0')
        ? "[^v] Nav  [Enter] Select Drive  [Esc] Back"
        : "[^v] Nav  [<-] Up  [->] Open  [B] Boot  [.] Dots  [Esc] Back";

    bt_gui_rect content;
    bt_gui_window(s, ctx, cw, ch,
                  title, sub,
                  footer_left, NULL,
                  &content);

    int x = 8;
    int y = content.y;
    int start = br->top;
    int count = arrlen(br->entries);
    int end = start + br->view_rows;
    if (end > count) end = count;

    if (count == 0) {
        const char *msg = (br->cwd[0] == '\0') ? "No drives found" : "(empty)";
        gfx_sprite_draw_str(s, x, y, msg, 150, 150, 180, 255, 16);
    }

    for (int i = start; i < end; i++) {
        const struct br_entry *e = &br->entries[i];

        if (i == br->cur)
            gfx_sprite_fill(s, 2, y, 472, BR_LINE_H, 50, 50, 120, 255);

        int icon_y = y + (BR_LINE_H - 24) / 2;
        bt_gui_icon_entry_t *icon = NULL;
        if (e->is_drive)
            icon = shgetp_null(br->icons, "disc");
        else if (e->is_dir)
            icon = shgetp_null(br->icons, "folder");
        else if (e->bootable)
            icon = shgetp_null(br->icons, "boot");
        else
            icon = shgetp_null(br->icons, "file");
        if (icon)
            gfx_sprite_draw_sprite(s, &icon->value, x, icon_y);

        int tx = x + 28;
        int text_y = y + (BR_LINE_H - 16) / 2;
        char trunc_name[BR_NAME_MAX];
        int max_chars = 48;
        if (strlen(e->name) > max_chars) {
            br_safe_strncpy(trunc_name, e->name, max_chars - 3);
            strcpy(trunc_name + max_chars - 3, "...");
        } else {
            strcpy(trunc_name, e->name);
        }

        if (e->is_drive) {
            gfx_sprite_draw_str(s, tx, text_y, trunc_name, 100, 255, 100, 255, 16);
        } else if (e->is_dir) {
            int len = strlen(trunc_name);
            trunc_name[len] = '/';
            trunc_name[len + 1] = '\0';
            gfx_sprite_draw_str(s, tx, text_y, trunc_name, 100, 200, 255, 255, 16);
        } else if (e->bootable) {
            gfx_sprite_draw_str(s, tx, text_y, trunc_name, 255, 200, 50, 255, 16);
        } else {
            gfx_sprite_draw_str(s, tx, text_y, trunc_name, 200, 200, 200, 255, 16);
        }

        y += BR_LINE_H;
    }

    /* Right column: info panel for the selected entry */
    int rx = 476;
    int ry = content.y;
    int rw = cw - 476 - 8;
    int rh = content.h - 8;

    gfx_sprite_fill(s, rx, ry, 1, rh, 80, 80, 120, 255);

    int card_x = rx + 12;
    int card_y = ry + 12;
    int card_w = rw - 24;
    int card_h = rh - 24;

    gfx_sprite_fill(s, card_x, card_y, card_w, card_h, 25, 25, 45, 255);
    gfx_sprite_fill(s, card_x, card_y, card_w, 1, 60, 60, 90, 255);
    gfx_sprite_fill(s, card_x, card_y + card_h - 1, card_w, 1, 60, 60, 90, 255);
    gfx_sprite_fill(s, card_x, card_y, 1, card_h, 60, 60, 90, 255);
    gfx_sprite_fill(s, card_x + card_w - 1, card_y, 1, card_h, 60, 60, 90, 255);

    if (br->cur >= 0 && br->cur < count) {
        const struct br_entry *e = &br->entries[br->cur];
        int tx = card_x + 12;
        int ty = card_y + 12;

        if (e->is_drive) {
            gfx_sprite_draw_str(s, tx, ty, "DRIVE INFO", 100, 255, 100, 255, 16);
            ty += 28;
            gfx_sprite_draw_str(s, tx, ty, "Name:", 180, 180, 200, 255, 16);
            ty += 22;
            gfx_sprite_draw_str(s, tx + 8, ty, e->name, 255, 255, 255, 255, 16);
            ty += 28;
            gfx_sprite_draw_str(s, tx, ty, "Type: Hardware Drive", 180, 180, 200, 255, 16);
            ty += 36;
            gfx_sprite_draw_str(s, tx, ty, "Action:", 180, 180, 200, 255, 16);
            ty += 22;
            gfx_sprite_draw_str(s, tx + 8, ty, "Press [Enter] to open drive.", 200, 200, 200, 255, 16);
        } else if (e->is_dir) {
            gfx_sprite_draw_str(s, tx, ty, "FOLDER INFO", 100, 200, 255, 255, 16);
            ty += 28;
            gfx_sprite_draw_str(s, tx, ty, "Name:", 180, 180, 200, 255, 16);
            ty += 22;
            char short_name[32];
            int max_chars = (card_w - 24) / 6;
            if (max_chars > 31) max_chars = 31;
            int name_len = strlen(e->name);
            if (name_len > max_chars) {
                br_safe_strncpy(short_name, e->name, max_chars - 3);
                strcpy(short_name + max_chars - 3, "...");
            } else {
                strcpy(short_name, e->name);
            }
            gfx_sprite_draw_str(s, tx + 8, ty, short_name, 255, 255, 255, 255, 16);
            ty += 28;
            gfx_sprite_draw_str(s, tx, ty, "Type: Directory", 180, 180, 200, 255, 16);
            ty += 36;
            gfx_sprite_draw_str(s, tx, ty, "Action:", 180, 180, 200, 255, 16);
            ty += 22;
            gfx_sprite_draw_str(s, tx + 8, ty, "Press [Enter] to open directory.", 200, 200, 200, 255, 16);
        } else {
            if (e->bootable) {
                gfx_sprite_draw_str(s, tx, ty, "BOOTABLE FILE", 255, 200, 50, 255, 16);
            } else {
                gfx_sprite_draw_str(s, tx, ty, "FILE INFO", 180, 180, 200, 255, 16);
            }
            ty += 28;
            gfx_sprite_draw_str(s, tx, ty, "Name:", 180, 180, 200, 255, 16);
            ty += 22;
            char short_name[32];
            int max_chars = (card_w - 24) / 6;
            if (max_chars > 31) max_chars = 31;
            int name_len = strlen(e->name);
            if (name_len > max_chars) {
                br_safe_strncpy(short_name, e->name, max_chars - 3);
                strcpy(short_name + max_chars - 3, "...");
            } else {
                strcpy(short_name, e->name);
            }
            gfx_sprite_draw_str(s, tx + 8, ty, short_name, 255, 255, 255, 255, 16);
            ty += 28;

            gfx_sprite_draw_str(s, tx, ty, "Size:", 180, 180, 200, 255, 16);
            ty += 22;
            char size_str[32];
            if (e->size == (unsigned long long)-1 || e->size == 0) {
                strcpy(size_str, "0 bytes");
            } else if (e->size >= 1024 * 1024 * 1024) {
                int gb = e->size / (1024 * 1024 * 1024);
                int mb = (e->size % (1024 * 1024 * 1024)) / (1024 * 1024 * 10);
                sprintf(size_str, "%d.%02d GB", gb, mb);
            } else if (e->size >= 1024 * 1024) {
                int mb = e->size / (1024 * 1024);
                int kb = (e->size % (1024 * 1024)) / (1024 * 10);
                sprintf(size_str, "%d.%02d MB", mb, kb);
            } else if (e->size >= 1024) {
                int kb = e->size / 1024;
                int b = (e->size % 1024) / 10;
                sprintf(size_str, "%d.%02d KB", kb, b);
            } else {
                sprintf(size_str, "%u bytes", (unsigned)e->size);
            }
            gfx_sprite_draw_str(s, tx + 8, ty, size_str, 200, 255, 200, 255, 16);
            ty += 28;

            gfx_sprite_draw_str(s, tx, ty, "Status:", 180, 180, 200, 255, 16);
            ty += 22;
            if (e->bootable) {
                gfx_sprite_draw_str(s, tx + 8, ty, "Bootable [YES]", 100, 255, 100, 255, 16);
                ty += 28;
                gfx_sprite_draw_str(s, tx, ty, "Action:", 180, 180, 200, 255, 16);
                ty += 22;
                gfx_sprite_draw_str(s, tx + 8, ty, "Press [Enter] or [B] to boot.", 200, 200, 200, 255, 16);
            } else {
                gfx_sprite_draw_str(s, tx + 8, ty, "Non-bootable [NO]", 255, 100, 100, 255, 16);
                ty += 28;
                gfx_sprite_draw_str(s, tx, ty, "Action:", 180, 180, 200, 255, 16);
                ty += 22;
                gfx_sprite_draw_str(s, tx + 8, ty, "No direct boot script.", 150, 150, 150, 255, 16);
            }
        }
    }
}

static void br_go_up(struct browser *br) {
    if (br->cwd[0] == '\0') return;

    if (strcmp(br->cwd, "/") == 0) {
        br->cwd[0] = '\0';
        br->device[0] = '\0';
        br_list_dir(br);
        return;
    }

    int len = strlen(br->cwd);
    if (len <= 1) return;

    if (br->cwd[len - 1] == '/')
        br->cwd[--len] = '\0';

    int i = len - 1;
    while (i > 0 && br->cwd[i] != '/') i--;

    if (i == 0) {
        strcpy(br->cwd, "/");
    } else {
        br->cwd[i + 1] = '\0';
    }

    br_list_dir(br);
}

static void br_enter_dir(struct browser *br, const char *name) {
    int len = strlen(br->cwd);

    if (len > 0 && len + 1 < BR_PATH_MAX && br->cwd[len - 1] != '/') {
        br->cwd[len++] = '/';
        br->cwd[len] = '\0';
    }

    strcpy(br->cwd + len, name);

    len = strlen(br->cwd);
    if (len > 0 && len + 1 < BR_PATH_MAX && br->cwd[len - 1] != '/') {
        br->cwd[len++] = '/';
        br->cwd[len] = '\0';
    }

    br_list_dir(br);
}

static int br_run_boot_cmd(const char *drive, const char *path) {
    char cmd[BR_PATH_MAX + 256];
    int plen = strlen(path);
    if (plen >= 4 && strnicmp(path + plen - 4, ".efi", 4) == 0) {
        sprintf(cmd, "chainloader %s%s ;; boot", drive, path);
    } else if (plen >= 4 && strnicmp(path + plen - 4, ".wim", 4) == 0) {
        sprintf(cmd,
                "root %s ;; uuid () ;; "
                "kernel /ntloader hires=no uuid=%%?_UUID%% wim=%s ;; "
                "initrd /initrd.cpio ;; boot",
                drive, path);
    } else {
        if (plen >= 4 &&
            (strnicmp(path + plen - 4, ".ima", 4) == 0 ||
             strnicmp(path + plen - 4, ".img", 4) == 0)) {
            sprintf(cmd, "map --mem %s%s (fd0) ;; map --hook ;; root (fd0) ;; chainloader +1 ;; boot",
                    drive, path);
        } else {
            sprintf(cmd, "map %s%s (0xff) ;; map --hook ;; chainloader (0xff) ;; boot",
                    drive, path);
        }
    }
    int r = run_line(cmd, BUILTIN_CMDLINE);
    if (errnum)
        return errnum;
    return r;
}

static void br_boot_file(const struct browser *br,
                         struct gfx *ctx, int cw, int ch) {
    const struct br_entry *e = &br->entries[br->cur];
    if (!e->bootable) {
        return;
    }

    char path[BR_PATH_MAX * 2];
    strcpy(path, br->cwd);
    int plen = strlen(path);
    if (plen > 0 && path[plen - 1] != '/') {
        path[plen++] = '/';
        path[plen] = '\0';
    }
    const char *src = e->name;
    while (*src) {
        if (*src == ' ' || *src == '"' || *src == '\\')
            path[plen++] = '\\';
        path[plen++] = *src++;
    }
    path[plen] = '\0';

    bt_gui_boot_feedback(ctx, cw, ch, FOOTER_H, "Booting...", path);
    gfx_flush(ctx);

    int ret = br_run_boot_cmd(br->device, path);
    if (ret != 0) {
        bt_gui_boot_feedback(ctx, cw, ch, FOOTER_H, "Boot failed", path);
        gfx_flush(ctx);
        gfx_getkey(ctx);
    }
}

/* Run the browser over the current menu screen. `target` is the item's
   target path: "(dev)/dir", a plain "/dir" (resolved against %@root%), or
   empty to start at the drive list. Returns when the user presses Esc, and
   the caller redraws the menu view it was opened from. */
static void br_run(struct gfx *g, int cw, int ch,
                   bt_gui_icon_entry_t *icons, const char *target) {
    struct gfx_sprite *screen = gfx_screen(g);
    struct browser br;

    memset(&br, 0, sizeof(br));
    br.icons = icons;
    br.view_rows = (ch - BR_HEADER_H - FOOTER_H) / BR_LINE_H;

    if (target && target[0]) {
        br_parse_device_path(target, br.device, br.cwd);
        int clen = strlen(br.cwd);
        if (clen > 0 && clen + 1 < BR_PATH_MAX && br.cwd[clen - 1] != '/') {
            br.cwd[clen++] = '/';
            br.cwd[clen] = '\0';
        }
    }

    while (1) {
        if (br_list_dir(&br) != 0) {
            gfx_sprite_clear(screen, 15, 15, 30, 255);
            if (br.cwd[0] == '\0') {
                gfx_sprite_draw_str(screen, 8, ch / 2 - 12,
                                    "No drives found", 255, 50, 50, 255, 24);
                gfx_flush(g);
                gfx_getkey(g);
                arrfree(br.entries);
                return;
            }
            gfx_sprite_draw_str(screen, 8, ch / 2 - 12,
                                "Cannot list directory", 255, 50, 50, 255, 24);
            gfx_flush(g);
            gfx_getkey(g);
            br.cwd[0] = '\0';
            br.device[0] = '\0';
            continue;
        }

        if (br.cwd[0] == '\0' && arrlen(br.entries) == 0) {
            gfx_sprite_clear(screen, 15, 15, 30, 255);
            gfx_sprite_draw_str(screen, 8, ch / 2 - 12,
                                "No drives found", 255, 50, 50, 255, 24);
            gfx_flush(g);
            gfx_getkey(g);
            arrfree(br.entries);
            return;
        }

        int opened_drive = 0;
        while (1) {
            int count = arrlen(br.entries);

            br_load_selected_size(&br);

            gfx_sprite_clear(screen, 15, 15, 30, 255);
            br_draw(&br, screen, g, cw, ch);
            gfx_flush(g);

            int key = gfx_getkey(g);
            int scan = (key >> 8) & 0xFF;
            int ascii = key & 0xFF;

            /* Esc always leaves the browser and returns to the menu view. */
            if (ascii == 0x1B || ascii == 'q' || ascii == 'Q') {
                arrfree(br.entries);
                return;
            } else if (ascii == 0x0D || scan == 0x4D) {
                if (br.cur < count) {
                    struct br_entry *e = &br.entries[br.cur];
                    if (e->is_dir) {
                        br_enter_dir(&br, e->name);
                    } else if (e->is_drive) {
                        strcpy(br.device, e->name);
                        strcpy(br.cwd, "/");
                        opened_drive = 1;
                        break;
                    } else if (e->bootable) {
                        br_boot_file(&br, g, cw, ch);
                    }
                }
            } else if (ascii == 'b' || ascii == 'B') {
                if (br.cur < count && br.entries[br.cur].bootable)
                    br_boot_file(&br, g, cw, ch);
            } else if (scan == 0x4B) {
                br_go_up(&br);
            } else if (ascii == '.') {
                if (br.cwd[0] != '\0') {
                    br.show_dotfiles = !br.show_dotfiles;
                    br_list_dir(&br);
                }
            } else if (scan == 0x48) {
                br.cur--;
                br_ensure_visible(&br);
            } else if (scan == 0x50) {
                br.cur++;
                br_ensure_visible(&br);
            } else if (scan == 0x47) {
                br.cur = 0;
                br_ensure_visible(&br);
            } else if (scan == 0x4F) {
                br.cur = arrlen(br.entries) - 1;
                br_ensure_visible(&br);
            }
        }
        (void)opened_drive; /* the outer loop re-lists the newly opened drive */
    }
}

static int action_type_from_name(const char *name) {
    if (!name) return ACTION_NONE;
    if (stricmp(name, "disk-image") == 0) return ACTION_DISK_IMAGE;
    if (stricmp(name, "file-browser") == 0) return ACTION_FILE_BROWSER;
    if (stricmp(name, "chainload") == 0) return ACTION_CHAINLOAD;
    if (stricmp(name, "reboot") == 0) return ACTION_REBOOT;
    if (stricmp(name, "poweroff") == 0) return ACTION_POWEROFF;
    if (stricmp(name, "open-category") == 0) return ACTION_OPEN_CATEGORY;
    if (stricmp(name, "program") == 0) return ACTION_PROGRAM;
    if (stricmp(name, "boot-wim") == 0) return ACTION_BOOT_WIM;
    return ACTION_NONE;
}

static void expand_wildcard_items(struct menu *m,
                                   const struct bt_ini_section *sec,
                                   int type, const char *title_template,
                                   const char *category, const char *target) {
    const char *slash = NULL;
    {
        const char *sp = target;
        while (*sp) { if (*sp == '/') slash = sp; sp++; }
    }
    if (!slash) return;

    int dir_len = (int)(slash - target);
    char dir_path[PATH_MAX];
    if (dir_len == 0) {
        dir_path[0] = '/';
        dir_path[1] = '\0';
    } else {
        memcpy(dir_path, target, dir_len);
        dir_path[dir_len] = '\0';
    }
    /* Ensure trailing slash so GRUB4DOS dir treats it as a directory */
    {
        int dl = strlen(dir_path);
        if (dl > 0 && dir_path[dl - 1] != '/') {
            dir_path[dl] = '/';
            dir_path[dl + 1] = '\0';
        }
    }
    const char *pattern = slash + 1;

    struct bt_dir_entry *entries = bt_directory_list(dir_path);
    if (!entries) return;

    for (int i = 0; entries[i].name; i++) {
        if (entries[i].name[0] == '.') continue;
        if (!bt_fnmatch(pattern, entries[i].name))
            continue;

        char full_path[PATH_MAX];
        int flen = strlen(dir_path);
        memcpy(full_path, dir_path, flen);
        if (flen > 0 && dir_path[flen - 1] != '/')
            full_path[flen++] = '/';
        int nlen = strlen(entries[i].name);
        memcpy(full_path + flen, entries[i].name, nlen + 1);

        struct menu_item *item = arraddnptr(m->items, 1);
        memset(item, 0, sizeof(*item));

        bt_token_t toks[] = {
            {"basename", entries[i].name},
            {"path", full_path},
        };
        bt_token_replace(item->title, sizeof(item->title), title_template,
                         toks, 2);

        const char *desc = bt_ini_section_get_value(sec, "desc");
        if (desc)
            bt_token_replace(item->desc, sizeof(item->desc), desc,
                             toks, 2);

        if (category && category[0])
            strcpy(item->category, category);
        else
            strcpy(item->category, "default");

        item->action.type = type;
        strcpy(item->action.target, full_path);

        char type_icon[24] = "";
        switch (type) {
        case ACTION_DISK_IMAGE:   strcpy(type_icon, "disc");     break;
        case ACTION_FILE_BROWSER: strcpy(type_icon, "folder");   break;
        case ACTION_CHAINLOAD:    strcpy(type_icon, "boot");     break;
        case ACTION_REBOOT:       strcpy(type_icon, "restart");  break;
        case ACTION_POWEROFF:     strcpy(type_icon, "poweroff"); break;
        case ACTION_OPEN_CATEGORY: strcpy(type_icon, "menu");    break;
        case ACTION_PROGRAM:       strcpy(type_icon, "console"); break;
        case ACTION_BOOT_WIM:      strcpy(type_icon, "windows"); break;
        }

        const char *custom_icon = bt_ini_section_get_value(sec, "icon");
        if (custom_icon && shgetp_null(m->icons, custom_icon))
            strcpy(item->icon_name, custom_icon);
        else
            strcpy(item->icon_name, type_icon);
    }

    for (int i = 0; entries[i].name; i++)
        free(entries[i].name);
    free(entries);
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

        /* Skip items from other categories if not loaded yet */
        const char *category = bt_ini_section_get_value(&ini.sections[i], "category");

        /* open-category items need a non-empty target category */
        const char *target = bt_ini_section_get_value(&ini.sections[i], "target");
        if (type == ACTION_OPEN_CATEGORY && (!target || !target[0]))
            continue;

        /* Firmware-specific filtering */
        const char *if_firmware = bt_ini_section_get_value(&ini.sections[i], "if_firmware");
        if (if_firmware) {
            if (stricmp(if_firmware, "bios") == 0 && !is_bios) continue;
            if (stricmp(if_firmware, "uefi") == 0 && is_bios) continue;
        }

        /* Wildcard expansion: target contains * or ? */
        if (target) {
            const char *wp = target;
            int has_wild = 0;
            while (*wp) { if (*wp == '*' || *wp == '?') { has_wild = 1; break; } wp++; }
            if (has_wild) {
                expand_wildcard_items(m, &ini.sections[i], type, title, category, target);
                continue;
            }
        }

        struct menu_item *item = arraddnptr(m->items, 1);
        memset(item, 0, sizeof(*item));

        strcpy(item->title, title);

        if (category && category[0]) {
            strcpy(item->category, category);
        } else {
            strcpy(item->category, "default");
        }

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
        case ACTION_OPEN_CATEGORY: strcpy(type_icon, "menu");    break;
        case ACTION_PROGRAM:       strcpy(type_icon, "console"); break;
        case ACTION_BOOT_WIM:      strcpy(type_icon, "windows"); break;
        }

        const char *custom_icon = bt_ini_section_get_value(&ini.sections[i], "icon");
        if (custom_icon && shgetp_null(m->icons, custom_icon)) {
            strcpy(item->icon_name, custom_icon);
        } else {
            strcpy(item->icon_name, type_icon);
        }

        if (target)
            strcpy(item->action.target, target);


    }

    m->confirm_exit = bt_ini_get_bool(&ini, "menu", "confirm_exit", 1);
    bt_ini_destroy(&ini);
}

static void rebuild_view(struct menu *m) {
    arrsetlen(m->view, 0);
    for (int i = 0; i < arrlen(m->items); i++) {
        if (strcmp(m->items[i].category, m->current_category) == 0) {
            arraddnptr(m->view, 1);
            m->view[arrlen(m->view) - 1] = i;
        }
    }
}

int gmain(int argc, char *argv[], int flags) {
    (void)argc;
    (void)argv;
#if defined(__i386__)
    is_bios = 1;
#else
    is_bios = 0;
#endif

    struct gfx g;
    if (!gfx_init(&g))
        return 1;

    int cw, ch, pad_x, pad_y;
    bt_gui_canvas(g.width, g.height, CANVAS_W, CANVAS_H, &cw, &ch, &pad_x, &pad_y);

    struct menu *m = malloc(sizeof(struct menu));
    if (!m) {
        gfx_close(&g);
        return 1;
    }
    boot_cmd = zalloc(BOOT_CMD_SIZE);
    boot_log = zalloc(BOOT_LOG_SIZE);
    if (!boot_cmd || !boot_log) {
        free(boot_cmd);
        free(boot_log);
        free(m);
        gfx_close(&g);
        return 1;
    }
    memset(m, 0, sizeof(struct menu));
    m->view_rows = (ch - HEADER_H - FOOTER_H) / LINE_H;
    m->current_category = malloc(8);
    if (m->current_category)
        strcpy(m->current_category, "default");
    m->current_category_display = malloc(42);
    if (m->current_category_display)
        strcpy(m->current_category_display, "Select the item you want to continue with");

    bt_gui_icon_load(&m->icons, "disc", ICON_DISC_24_PNG);
    bt_gui_icon_load(&m->icons, "folder", ICON_FOLDER_24_PNG);
    bt_gui_icon_load(&m->icons, "file", ICON_FILE_24_PNG);
    bt_gui_icon_load(&m->icons, "boot", ICON_BOOT_24_PNG);
    bt_gui_icon_load(&m->icons, "restart", ICON_RESTART_24_PNG);
    bt_gui_icon_load(&m->icons, "poweroff", ICON_POWEROFF_24_PNG);
    bt_gui_icon_load(&m->icons, "windows", ICON_WINDOWS_24_PNG);
    bt_gui_icon_load(&m->icons, "menu", ICON_MENU_24_PNG);
    bt_gui_icon_load(&m->icons, "console", ICON_CONSOLE_24_PNG);
    bt_gui_icon_load(&m->icons, "broken_robot", ICON_BROKEN_ROBOT_50_PNG);

    load_ini_items(m);
    rebuild_view(m);

    struct gfx_sprite *screen = gfx_screen(&g);
    while (1) {
        gfx_sprite_clear(screen, 15, 15, 30, 255);
        draw(m, screen, &g, cw, ch);
        gfx_flush(&g);

        int key = gfx_getkey(&g);
        int scan = (key >> 8) & 0xFF;
        int ascii = key & 0xFF;

        if (ascii == 0x1B || ascii == 'q' || ascii == 'Q') {
            if (arrlen(m->category_stack) > 0) {
                free(m->current_category);
                free(m->current_category_display);
                struct cat_nav *e = &m->category_stack[arrlen(m->category_stack) - 1];
                m->current_category = e->category;
                m->current_category_display = e->display;
                arrpop(m->category_stack);
                rebuild_view(m);
                m->cur = 0;
                m->top = 0;
            } else {
                 if (!m->confirm_exit || bt_gui_confirm(&g, cw, ch, "Quit Boot Menu?", NULL))
                    goto done;
            }
        } else if (ascii == 0x0D) {
            int count = arrlenu(m->view);
            if (m->cur >= 0 && m->cur < count) {
                struct menu_item *item = &m->items[m->view[m->cur]];
                switch (item->action.type) {
                case ACTION_DISK_IMAGE:
                    handle_disk_image(&g, cw, ch,
                                      item->action.target);
                    break;
                case ACTION_FILE_BROWSER:
                    br_run(&g, cw, ch, m->icons, item->action.target);
                    break;
                case ACTION_CHAINLOAD:
                    handle_chainload(&g, cw, ch,
                                     item->action.target);
                    break;
                case ACTION_REBOOT:
                    handle_reboot(&g, cw, ch);
                    break;
                case ACTION_POWEROFF:
                    handle_poweroff(&g, cw, ch);
                    break;
                case ACTION_OPEN_CATEGORY:
                    {
                        struct cat_nav *e = arraddnptr(m->category_stack, 1);
                        e->category = m->current_category;
                        e->display  = m->current_category_display;
                        m->current_category = malloc(strlen(item->action.target) + 1);
                        if (m->current_category)
                            strcpy(m->current_category, item->action.target);
                        m->current_category_display = malloc(strlen(item->title) + 1);
                        if (m->current_category_display)
                            strcpy(m->current_category_display, item->title);
                        rebuild_view(m);
                        m->cur = 0;
                        m->top = 0;
                    }
                    break;
                case ACTION_PROGRAM:
                    {
                        char cmd[512];
                        sprintf(cmd, "%%moddir%%/%s", item->action.target);
                        run_line(cmd, BUILTIN_CMDLINE);
                    }
                    break;
                case ACTION_BOOT_WIM:
                    handle_boot_wim(&g, cw, ch,
                                    item->action.target);
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
            m->cur = (int)arrlenu(m->view) - 1;
            ensure_visible(m);
        } else if (scan == 0x49) {
            m->cur -= m->view_rows;
            ensure_visible(m);
        } else if (scan == 0x51) {
            m->cur += m->view_rows;
            ensure_visible(m);
        }
    }

done:
    for (int i = 0; i < arrlen(m->category_stack); i++) {
        free(m->category_stack[i].category);
        free(m->category_stack[i].display);
    }
    arrfree(m->category_stack);
    arrfree(m->view);
    free(m->current_category);
    free(m->current_category_display);
    bt_gui_icons_destroy(&m->icons);
    arrfree(m->items);
    free(boot_cmd);
    free(boot_log);
    free(m);
    gfx_close(&g);
    return 0;
}
