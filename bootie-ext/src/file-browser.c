#include <bootie.h>
#include <bootie-io.h>
#include <bootie-gfx.h>
#include <stdint.h>

#define MAX_ENTRIES 512
#define PATH_MAX 260
#define NAME_MAX 128
#define BUF_CAP 131072

#define LINE_H 9
#define HEADER_H 24
#define FOOTER_H 12
#define MAX_DRIVES 32
#define DPAD_Y 4
#define DPAD_LH 9

static const char *bootable_ext[] = {
    ".iso", ".img", ".ima", ".vhd", ".vhdx", ".wim", ".efi", NULL
};

struct entry {
    char name[NAME_MAX];
    int is_dir;
    int bootable;
};

struct browser {
    char cwd[PATH_MAX];
    struct entry entries[MAX_ENTRIES];
    int count;
    int top;
    int cur;
    int view_rows;
    int show_dotfiles;
};

static int has_boot_ext(const char *name) {
    int len = strlen(name);
    for (int i = 0; bootable_ext[i]; i++) {
        int elen = strlen(bootable_ext[i]);
        if (len >= elen && strncmpx(name + len - elen, bootable_ext[i], elen, 1) == 0)
            return 1;
    }
    return 0;
}

static int list_dir(struct browser *br) {
    char *buf = malloc(BUF_CAP);
    if (!buf) return -1;

    errnum = ERR_NONE;
    uintptr_t saved = putchar_hooked;
    putchar_hooked = (uintptr_t)buf;

    bt_dir_list(br->cwd);

    char *end = (char *)putchar_hooked;
    putchar_hooked = saved;

    if (errnum != ERR_NONE) {
        free(buf);
        return -1;
    }

    if (end >= buf && end < buf + BUF_CAP)
        *end = '\0';
    else
        buf[BUF_CAP - 1] = '\0';

    br->count = 0;
    br->top = 0;
    br->cur = 0;

    int is_root = (strcmp(br->cwd, "/") == 0);
    if (!is_root) {
        struct entry *e = &br->entries[br->count++];
        strcpy(e->name, "..");
        e->is_dir = 1;
        e->bootable = 0;
    }

    char *p = buf;
    while (*p && br->count < MAX_ENTRIES) {
        while (*p == ' ') p++;
        if (!*p) break;

        char name[NAME_MAX];
        int n = 0;

        while (*p && *p != ' ') {
            if (*p == '\\') {
                p++;
                if (*p) name[n++] = *p++;
            } else {
                name[n++] = *p++;
            }
        }
        name[n] = '\0';

        int is_dir = 0;
        if (n > 0 && name[n - 1] == '/') {
            name[--n] = '\0';
            is_dir = 1;
        }

        if (n > 0 && (br->show_dotfiles || name[0] != '.')) {
            struct entry *e = &br->entries[br->count++];
            strcpy(e->name, name);
            e->is_dir = is_dir;
            e->bootable = !is_dir && has_boot_ext(name);
        }
    }

    free(buf);

    // Second pass: use bt_file_open to distinguish files from directories
    // (some GRUB4DOS versions don't append '/' to directory names via grub_dir)
    for (int i = 0; i < br->count; i++) {
        struct entry *e = &br->entries[i];
        if (e->is_dir) continue;

        char fullpath[PATH_MAX];
        strcpy(fullpath, br->cwd);
        int plen = strlen(fullpath);
        if (plen > 0 && fullpath[plen - 1] != '/') {
            fullpath[plen++] = '/';
            fullpath[plen] = '\0';
        }
        strcpy(fullpath + plen, e->name);

        // open() returns 0 for directories, non-zero for files
        if (bt_file_open(fullpath) == 0) {
            e->is_dir = 1;
            e->bootable = 0;
            bt_file_close();
        }
    }

    return 0;
}

static void ensure_visible(struct browser *br) {
    if (br->count == 0) {
        br->top = 0;
        br->cur = 0;
        return;
    }
    if (br->cur < 0) br->cur = 0;
    if (br->cur >= br->count) br->cur = br->count - 1;
    if (br->top < 0) br->top = 0;
    if (br->top > br->cur) br->top = br->cur;
    if (br->cur >= br->top + br->view_rows)
        br->top = br->cur - br->view_rows + 1;
}

static void draw(const struct browser *br, struct gfx *g) {
    fill_rect(g, 0, 0, g->width, g->height, 15, 15, 30);

    draw_str(g, 8, 4, "File Browser", 200, 200, 255, 2);
    draw_str(g, 8 + 14 * 12, 8, br->cwd, 180, 180, 220, 1);

    fill_rect(g, 0, 22, g->width, 1, 80, 80, 120);

    int x = 8;
    int y = HEADER_H;
    int start = br->top;
    int end = start + br->view_rows;
    if (end > br->count) end = br->count;

    for (int i = start; i < end; i++) {
        const struct entry *e = &br->entries[i];

        if (i == br->cur)
            fill_rect(g, 0, y - 1, g->width, LINE_H, 50, 50, 120);

        char buf[NAME_MAX + 16];
        buf[0] = ' ';
        strcpy(buf + 1, e->name);

        if (e->is_dir) {
            int len = strlen(buf);
            buf[len] = '/';
            buf[len + 1] = '\0';
            draw_str(g, x, y, buf, 100, 200, 255, 1);
        } else if (e->bootable) {
            int len = strlen(buf);
            strcpy(buf + len, "  [BOOT]");
            draw_str(g, x, y, buf, 255, 200, 50, 1);
        } else {
            draw_str(g, x, y, buf, 200, 200, 200, 1);
        }

        y += LINE_H;
    }

    fill_rect(g, 0, g->height - FOOTER_H, g->width, FOOTER_H, 30, 30, 60);
    draw_str(g, x, g->height - FOOTER_H + 2,
             "[^v] Nav  [<-] Up  [->] Open  [B] Boot  [.] Dots  [Esc] Quit",
             150, 150, 180, 1);
}

static void go_up(struct browser *br) {
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

    list_dir(br);
}

static void enter_dir(struct browser *br, const char *name) {
    int len = strlen(br->cwd);

    if (len > 0 && br->cwd[len - 1] != '/') {
        br->cwd[len++] = '/';
        br->cwd[len] = '\0';
    }

    strcpy(br->cwd + len, name);

    len = strlen(br->cwd);
    if (len > 0 && br->cwd[len - 1] != '/') {
        br->cwd[len++] = '/';
        br->cwd[len] = '\0';
    }

    list_dir(br);
}

static void boot_file(const struct browser *br, struct gfx *g) {
    const struct entry *e = &br->entries[br->cur];
    if (!e->bootable) return;

    char path[PATH_MAX];
    strcpy(path, br->cwd);
    int len = strlen(path);
    if (len > 0 && path[len - 1] != '/') {
        path[len++] = '/';
        path[len] = '\0';
    }
    strcpy(path + len, e->name);

    fill_rect(g, 0, g->height - FOOTER_H * 2, g->width, FOOTER_H * 2, 40, 20, 20);
    draw_str(g, 8, g->height - FOOTER_H * 2 + 2, "Booting...", 255, 200, 50, 1);
    draw_str(g, 8, g->height - FOOTER_H + 2, path, 200, 200, 200, 1);
    gfx_getkey(g);
}

/* ------------------------------------------------------------------ */
/*  Drive picker                                                        */
/* ------------------------------------------------------------------ */

static int drive_picker(struct gfx *g, struct bt_drive_info *drives, int count) {
    int view = (g->height - DPAD_Y * 2 - DPAD_LH * 2) / DPAD_LH;
    if (view > count) view = count;
    if (view < 1) view = 1;

    int cur = 0;
    int top = 0;

    while (1) {
        fill_rect(g, 0, 0, g->width, g->height, 15, 15, 30);
        draw_str(g, 8, DPAD_Y, "Select Drive", 200, 200, 255, 2);
        fill_rect(g, 0, DPAD_Y + 18, g->width, 1, 80, 80, 120);

        int y = DPAD_Y + 24;
        int end = top + view;
        if (end > count) end = count;

        for (int i = top; i < end; i++) {
            if (i == cur)
                fill_rect(g, 0, y - 1, g->width, DPAD_LH, 50, 50, 120);

            char buf[32];
            buf[0] = ' ';
            strcpy(buf + 1, drives[i].name);
            draw_str(g, 8, y, buf, 200, 200, 200, 1);
            y += DPAD_LH;
        }

        fill_rect(g, 0, g->height - FOOTER_H, g->width, FOOTER_H, 30, 30, 60);
        draw_str(g, 8, g->height - FOOTER_H + 2,
                 "[^v] Nav  [Enter] Select  [Esc] Quit",
                 150, 150, 180, 1);

        int key = gfx_getkey(g);
        int scan = (key >> 8) & 0xFF;
        int ascii = key & 0xFF;

        if (ascii == 0x1B || ascii == 'q' || ascii == 'Q')
            return -1;
        else if (ascii == 0x0D)
            return cur;
        else if (scan == 0x48) {
            cur--;
            if (cur < 0) cur = 0;
            if (cur < top) top = cur;
        } else if (scan == 0x50) {
            cur++;
            if (cur >= count) cur = count - 1;
            if (cur >= top + view) top = cur - view + 1;
        }
    }
}

int gmain(int argc, char *argv[], int flags) {
    struct gfx g;
    if (!gfx_init(&g)) {
        printf("gfx_init failed\n");
        return 1;
    }

    struct browser *br = NULL;
    char start_device[24] = "";

    if (argc >= 2) {
        char *rest = bt_drive_set_dev(argv[1]);
        if (rest) {
            bt_drive_open_dev();
            saved_drive = current_drive;
            saved_partition = current_partition;
            strcpy(start_device, argv[1]);
            br = malloc(sizeof(struct browser));
            if (br) {
                br->view_rows = (g.height - HEADER_H - FOOTER_H) / LINE_H;
                br->show_dotfiles = 0;
                if (*rest == '/')
                    strcpy(br->cwd, rest);
                else
                    strcpy(br->cwd, "/");
            }
        }
    }

    while (1) {
        /* --- Drive picker (if no browser) --- */
        if (!br) {
            struct bt_drive_info drives[MAX_DRIVES];
            int nd = bt_drive_enum(drives, MAX_DRIVES);
            if (nd == 0) {
                draw_str(&g, 8, g.height / 2 - 7, "No drives found", 255, 50, 50, 2);
                gfx_getkey(&g);
                gfx_close(&g);
                return 1;
            }

            while (1) {
                int sel = drive_picker(&g, drives, nd);
                if (sel < 0) {
                    gfx_close(&g);
                    return 0;
                }

                strcpy(start_device, drives[sel].name);
                bt_drive_set_dev(start_device);
                bt_drive_open_dev();
                saved_drive = current_drive;
                saved_partition = current_partition;

                br = malloc(sizeof(struct browser));
                if (!br) continue;
                br->view_rows = (g.height - HEADER_H - FOOTER_H) / LINE_H;
                br->show_dotfiles = 0;
                strcpy(br->cwd, "/");

                if (list_dir(br) == 0)
                    break;

                free(br);
                br = NULL;

                fill_rect(&g, 0, g.height / 2 - 10, g.width, 30, 40, 20, 20);
                draw_str(&g, 8, g.height / 2 - 7, "Cannot access drive", 255, 100, 100, 2);
                gfx_getkey(&g);
            }
        }

        if (!br || list_dir(br) != 0) {
            draw_str(&g, 8, g.height / 2 - 7, "Cannot list directory", 255, 50, 50, 2);
            gfx_getkey(&g);
            gfx_close(&g);
            free(br);
            return 1;
        }

        /* --- File browser loop --- */
        while (1) {
            draw(br, &g);

            int key = gfx_getkey(&g);
            int scan = (key >> 8) & 0xFF;
            int ascii = key & 0xFF;

            if (ascii == 0x1B || ascii == 'q' || ascii == 'Q') {
                free(br);
                br = NULL;
                break;
            } else if (ascii == 'b' || ascii == 'B') {
                if (br->cur < br->count && br->entries[br->cur].bootable)
                    boot_file(br, &g);
            } else if (ascii == 0x0D || scan == 0x4D) {
                if (br->cur < br->count) {
                    const struct entry *e = &br->entries[br->cur];
                    if (strcmp(e->name, "..") == 0)
                        go_up(br);
                    else if (e->is_dir)
                        enter_dir(br, e->name);
                }
            } else if (scan == 0x4B) {
                go_up(br);
            } else if (ascii == '.') {
                br->show_dotfiles = !br->show_dotfiles;
                list_dir(br);
            } else if (scan == 0x48) {
                br->cur--;
                ensure_visible(br);
            } else if (scan == 0x50) {
                br->cur++;
                ensure_visible(br);
            }
        }

        if (!br)
            continue;

        break;
    }

    gfx_close(&g);
    return 0;
}
