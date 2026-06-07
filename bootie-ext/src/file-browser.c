#include <bootie.h>
#include <bootie-io.h>
#include <bootie-gfx.h>
#include <stdint.h>

#define MAX_ENTRIES 512
#define PATH_MAX 260
#define NAME_MAX 128
#define BUF_CAP 131072
#define MAX_DRIVES 32

#define LINE_H 16
#define HEADER_H 60
#define FOOTER_H 16
#define CANVAS_W 800
#define CANVAS_H 600

static const char *bootable_ext[] = {
    ".iso", ".img", ".ima", ".vhd", ".vhdx", ".wim", ".efi", NULL
};

struct entry {
    char name[NAME_MAX];
    int is_dir;
    int bootable;
    int is_drive;
};

struct browser {
    char cwd[PATH_MAX];
    char device[24];
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

static void sort_entries(struct entry *entries, int count) {
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
            struct entry tmp = entries[i];
            entries[i] = entries[best];
            entries[best] = tmp;
        }
    }
}

static int list_dir(struct browser *br) {
    if (br->cwd[0] == '\0') {
        struct bt_drive_info drives[MAX_DRIVES];
        int nd = bt_drive_enum(drives, MAX_DRIVES);
        br->count = 0;
        br->top = 0;
        br->cur = 0;
        for (int i = 0; i < nd && br->count < MAX_ENTRIES; i++) {
            struct entry *e = &br->entries[br->count++];
            strcpy(e->name, drives[i].name);
            e->is_dir = 0;
            e->bootable = 0;
            e->is_drive = 1;
        }
        sort_entries(br->entries, br->count);
        return 0;
    }

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

    {
        struct entry *e = &br->entries[br->count++];
        strcpy(e->name, "..");
        e->is_dir = 1;
        e->bootable = 0;
        e->is_drive = 0;
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
            e->is_drive = 0;
        }
    }

    free(buf);

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

        if (bt_file_open(fullpath) == 0) {
            e->is_dir = 1;
            e->bootable = 0;
            bt_file_close();
        }
    }

    if (br->count > 1)
        sort_entries(br->entries + 1, br->count - 1);

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

static void draw(const struct browser *br, struct gfx *g,
                 int px, int py, int cw, int ch) {
    fill_rect(g, 0, 0, g->width, g->height, 15, 15, 30);
    fill_rect(g, px, py, cw, ch, 15, 15, 30);

    fill_rect(g, px, py, cw, 1, 80, 80, 120);
    fill_rect(g, px, py + ch - 1, cw, 1, 80, 80, 120);
    fill_rect(g, px, py, 1, ch, 80, 80, 120);
    fill_rect(g, px + cw - 1, py, 1, ch, 80, 80, 120);

    const char *title = (br->cwd[0] == '\0') ? "Select Drive" : "File Browser";
    draw_str(g, px + 8, py + 4, title, 200, 200, 255, 2);

    if (br->cwd[0] == '\0') {
        draw_str(g, px + 8, py + 38, "Drives", 180, 180, 220, 1);
    } else if (br->device[0]) {
        char fullpath[PATH_MAX + 24];
        int dlen = strlen(br->device);
        strcpy(fullpath, br->device);
        strcpy(fullpath + dlen, br->cwd);
        draw_str(g, px + 8, py + 38, fullpath, 180, 180, 220, 1);
    } else {
        draw_str(g, px + 8, py + 38, br->cwd, 180, 180, 220, 1);
    }

    fill_rect(g, px, py + 56, cw, 1, 80, 80, 120);

    int x = px + 8;
    int y = py + HEADER_H;
    int start = br->top;
    int end = start + br->view_rows;
    if (end > br->count) end = br->count;

    if (br->count == 0) {
        const char *msg = (br->cwd[0] == '\0') ? "No drives found" : "(empty)";
        draw_str(g, x, y, msg, 150, 150, 180, 1);
    }

    for (int i = start; i < end; i++) {
        const struct entry *e = &br->entries[i];

        if (i == br->cur)
            fill_rect(g, px, y, cw, LINE_H, 50, 50, 120);

        char buf[NAME_MAX + 16];
        buf[0] = ' ';
        strcpy(buf + 1, e->name);

        if (e->is_drive) {
            int len = strlen(buf);
            strcpy(buf + len, "  [DRIVE]");
            draw_str(g, x, y + 2, buf, 100, 255, 100, 1);
        } else if (e->is_dir) {
            int len = strlen(buf);
            buf[len] = '/';
            buf[len + 1] = '\0';
            draw_str(g, x, y + 2, buf, 100, 200, 255, 1);
        } else if (e->bootable) {
            int len = strlen(buf);
            strcpy(buf + len, "  [BOOT]");
            draw_str(g, x, y + 2, buf, 255, 200, 50, 1);
        } else {
            draw_str(g, x, y + 2, buf, 200, 200, 200, 1);
        }

        y += LINE_H;
    }

    fill_rect(g, px, py + ch - FOOTER_H, cw, FOOTER_H, 30, 30, 60);

    if (br->cwd[0] == '\0') {
        draw_str(g, x, py + ch - FOOTER_H + 2,
                 "[^v] Nav  [Enter] Select Drive  [Esc] Quit",
                 150, 150, 180, 1);
    } else {
        draw_str(g, x, py + ch - FOOTER_H + 2,
                 "[^v] Nav  [<-] Up  [->] Open  [B] Boot  [.] Dots  [Esc] Back",
                 150, 150, 180, 1);
    }
}

static void go_up(struct browser *br) {
    if (br->cwd[0] == '\0') return;

    if (strcmp(br->cwd, "/") == 0) {
        br->cwd[0] = '\0';
        br->device[0] = '\0';
        list_dir(br);
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

static void boot_file(const struct browser *br, struct gfx *g,
                      int px, int py, int cw, int ch) {
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

    fill_rect(g, px, py + ch - FOOTER_H * 2, cw, FOOTER_H * 2, 40, 20, 20);
    draw_str(g, px + 8, py + ch - FOOTER_H * 2 + 2, "Booting...", 255, 200, 50, 1);
    draw_str(g, px + 8, py + ch - FOOTER_H + 2, path, 200, 200, 200, 1);
    gfx_getkey(g);
}

int gmain(int argc, char *argv[], int flags) {
    struct gfx g;
    if (!gfx_init(&g)) {
        return 1;
    }

    uint32_t fw = g.width;
    uint32_t fh = g.height;
    int canvas_w = fw < CANVAS_W ? (int)fw : CANVAS_W;
    int canvas_h = fh < CANVAS_H ? (int)fh : CANVAS_H;
    int pad_x = ((int)fw - canvas_w) / 2;
    int pad_y = ((int)fh - canvas_h) / 2;

    struct browser *br = malloc(sizeof(struct browser));
    if (!br) {
        gfx_close(&g);
        return 1;
    }
    br->view_rows = (canvas_h - HEADER_H - FOOTER_H) / LINE_H;
    br->show_dotfiles = 0;
    br->device[0] = '\0';

    if (argc >= 2) {
        char *rest = bt_drive_set_dev(argv[1]);
        if (rest) {
            bt_drive_open_dev();
            saved_drive = current_drive;
            saved_partition = current_partition;
            int devlen = rest - argv[1];
            if (devlen > 0 && devlen < (int)sizeof(br->device)) {
                int i;
                for (i = 0; i < devlen; i++)
                    br->device[i] = argv[1][i];
                br->device[devlen] = '\0';
            }
            if (*rest == '/')
                strcpy(br->cwd, rest);
            else
                strcpy(br->cwd, "/");
        } else {
            br->cwd[0] = '\0';
        }
    } else {
        br->cwd[0] = '\0';
    }

    while (1) {
        if (list_dir(br) != 0) {
            fill_rect(&g, 0, 0, g.width, g.height, 15, 15, 30);
            if (br->cwd[0] == '\0') {
                draw_str(&g, pad_x + 8, pad_y + canvas_h / 2 - 7,
                         "No drives found", 255, 50, 50, 2);
                gfx_getkey(&g);
                free(br);
                gfx_close(&g);
                return 1;
            }
            draw_str(&g, pad_x + 8, pad_y + canvas_h / 2 - 7,
                     "Cannot list directory", 255, 50, 50, 2);
            gfx_getkey(&g);
            br->cwd[0] = '\0';
            br->device[0] = '\0';
            continue;
        }

        if (br->cwd[0] == '\0' && br->count == 0) {
            fill_rect(&g, 0, 0, g.width, g.height, 15, 15, 30);
            draw_str(&g, pad_x + 8, pad_y + canvas_h / 2 - 7,
                     "No drives found", 255, 50, 50, 2);
            gfx_getkey(&g);
            free(br);
            gfx_close(&g);
            return 1;
        }

        while (1) {
            draw(br, &g, pad_x, pad_y, canvas_w, canvas_h);

            int key = gfx_getkey(&g);
            int scan = (key >> 8) & 0xFF;
            int ascii = key & 0xFF;

            if (ascii == 0x1B || ascii == 'q' || ascii == 'Q') {
                if (br->cwd[0] == '\0') {
                    free(br);
                    gfx_close(&g);
                    return 0;
                }
                br->cwd[0] = '\0';
                br->device[0] = '\0';
                break;
            } else if (ascii == 'b' || ascii == 'B') {
                if (br->cur < br->count && br->entries[br->cur].bootable)
                    boot_file(br, &g, pad_x, pad_y, canvas_w, canvas_h);
            } else if (ascii == 0x0D || scan == 0x4D) {
                if (br->cur < br->count) {
                    struct entry *e = &br->entries[br->cur];
                    if (strcmp(e->name, "..") == 0) {
                        go_up(br);
                    } else if (e->is_dir) {
                        enter_dir(br, e->name);
                    } else if (e->is_drive) {
                        bt_drive_set_dev(e->name);
                        bt_drive_open_dev();
                        saved_drive = current_drive;
                        saved_partition = current_partition;
                        strcpy(br->device, e->name);
                        strcpy(br->cwd, "/");
                        break;
                    }
                }
            } else if (scan == 0x4B) {
                go_up(br);
            } else if (ascii == '.') {
                if (br->cwd[0] != '\0') {
                    br->show_dotfiles = !br->show_dotfiles;
                    list_dir(br);
                }
            } else if (scan == 0x48) {
                br->cur--;
                ensure_visible(br);
            } else if (scan == 0x50) {
                br->cur++;
                ensure_visible(br);
            }
        }
    }
}
