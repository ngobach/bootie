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

#define NUM_BOOT_EXT 7
static const char bootable_ext[NUM_BOOT_EXT][8] = {
    ".iso", ".img", ".ima", ".vhd", ".vhdx", ".wim", ".efi"
};

struct entry {
    char name[NAME_MAX];
    int is_dir;
    int bootable;
    int is_drive;
    unsigned long long size;
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

static void safe_strncpy(char *dest, const char *src, int n) {
    int i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

static int has_boot_ext(const char *name) {
    int len = strlen(name);
    for (int i = 0; i < NUM_BOOT_EXT; i++) {
        const char *ext = bootable_ext[i];
        int elen = strlen(ext);
        if (len >= elen && strnicmp(name + len - elen, ext, elen) == 0)
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
            e->size = 0;
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
            e->size = 0;
        }
    }

    free(buf);
    for (int i = 0; i < br->count; i++) {
        struct entry *e = &br->entries[i];
        if (e->is_dir) continue;

        char fullpath[PATH_MAX * 2];
        strcpy(fullpath, br->cwd);
        int plen = strlen(fullpath);
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

        if (bt_file_open(fullpath) == 0) {
            e->is_dir = 1;
            e->bootable = 0;
            e->size = 0;
            bt_file_close();
        } else {
            bt_file_close();
        }
    }

    if (br->count > 1)
        sort_entries(br->entries, br->count);

    return 0;
}

static void load_selected_size(struct browser *br) {
    if (br->cur < 0 || br->cur >= br->count) return;
    struct entry *e = &br->entries[br->cur];
    if (e->is_dir || e->is_drive || e->size > 0 || e->size == (unsigned long long)-1) return;

    char fullpath[PATH_MAX * 2];
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

    unsigned long long size = bt_file_get_size(fullpath);
    if (size > 0) {
        e->size = size;
    } else {
        e->size = (unsigned long long)-1;
    }
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
    draw_str(g, px + 8, py + 12, title, 200, 200, 255, 2);

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
            fill_rect(g, px + 2, y, 472, LINE_H, 50, 50, 120);

        char buf[NAME_MAX + 32];
        buf[0] = ' ';
        
        char trunc_name[NAME_MAX];
        int max_left_chars = 52;
        if (strlen(e->name) > max_left_chars) {
            safe_strncpy(trunc_name, e->name, max_left_chars - 3);
            strcpy(trunc_name + max_left_chars - 3, "...");
        } else {
            strcpy(trunc_name, e->name);
        }
        strcpy(buf + 1, trunc_name);

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
            draw_str(g, x, y + 2, buf, 255, 200, 50, 1);
        } else {
            draw_str(g, x, y + 2, buf, 200, 200, 200, 1);
        }

        y += LINE_H;
    }

    // Right Column Info Panel
    int rx = px + 476;
    int ry = py + HEADER_H;
    int rw = cw - 476 - 8;
    int rh = ch - HEADER_H - FOOTER_H - 8;

    // Draw vertical dividing line
    fill_rect(g, rx, ry, 1, rh, 80, 80, 120);

    // Draw info card
    int card_x = rx + 12;
    int card_y = ry + 12;
    int card_w = rw - 24;
    int card_h = rh - 24;

    fill_rect(g, card_x, card_y, card_w, card_h, 25, 25, 45);
    fill_rect(g, card_x, card_y, card_w, 1, 60, 60, 90);
    fill_rect(g, card_x, card_y + card_h - 1, card_w, 1, 60, 60, 90);
    fill_rect(g, card_x, card_y, 1, card_h, 60, 60, 90);
    fill_rect(g, card_x + card_w - 1, card_y, 1, card_h, 60, 60, 90);

    if (br->cur >= 0 && br->cur < br->count) {
        const struct entry *e = &br->entries[br->cur];
        int tx = card_x + 12;
        int ty = card_y + 12;

        if (e->is_drive) {
            draw_str(g, tx, ty, "DRIVE INFO", 100, 255, 100, 1);
            ty += 24;
            draw_str(g, tx, ty, "Name:", 180, 180, 200, 1);
            ty += 16;
            draw_str(g, tx + 8, ty, e->name, 255, 255, 255, 1);
            ty += 24;
            draw_str(g, tx, ty, "Type: Hardware Drive", 180, 180, 200, 1);
            ty += 32;
            draw_str(g, tx, ty, "Action:", 180, 180, 200, 1);
            ty += 16;
            draw_str(g, tx + 8, ty, "Press [Enter] to open drive.", 200, 200, 200, 1);
        } else if (e->is_dir) {
            draw_str(g, tx, ty, "FOLDER INFO", 100, 200, 255, 1);
            ty += 24;
            draw_str(g, tx, ty, "Name:", 180, 180, 200, 1);
            ty += 16;
            char short_name[32];
            int max_chars = (card_w - 24) / 6;
            if (max_chars > 31) max_chars = 31;
            int name_len = strlen(e->name);
            if (name_len > max_chars) {
                safe_strncpy(short_name, e->name, max_chars - 3);
                strcpy(short_name + max_chars - 3, "...");
            } else {
                strcpy(short_name, e->name);
            }
            draw_str(g, tx + 8, ty, short_name, 255, 255, 255, 1);
            ty += 24;
            draw_str(g, tx, ty, "Type: Directory", 180, 180, 200, 1);
            ty += 32;
            draw_str(g, tx, ty, "Action:", 180, 180, 200, 1);
            ty += 16;
            draw_str(g, tx + 8, ty, "Press [Enter] to open directory.", 200, 200, 200, 1);
        } else {
            if (e->bootable) {
                draw_str(g, tx, ty, "BOOTABLE FILE", 255, 200, 50, 1);
            } else {
                draw_str(g, tx, ty, "FILE INFO", 180, 180, 200, 1);
            }
            ty += 24;
            draw_str(g, tx, ty, "Name:", 180, 180, 200, 1);
            ty += 16;
            char short_name[32];
            int max_chars = (card_w - 24) / 6;
            if (max_chars > 31) max_chars = 31;
            int name_len = strlen(e->name);
            if (name_len > max_chars) {
                safe_strncpy(short_name, e->name, max_chars - 3);
                strcpy(short_name + max_chars - 3, "...");
            } else {
                strcpy(short_name, e->name);
            }
            draw_str(g, tx + 8, ty, short_name, 255, 255, 255, 1);
            ty += 24;

            draw_str(g, tx, ty, "Size:", 180, 180, 200, 1);
            ty += 16;
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
                sprintf(size_str, "%d bytes", (int)e->size);
            }
            draw_str(g, tx + 8, ty, size_str, 200, 255, 200, 1);
            ty += 24;

            draw_str(g, tx, ty, "Status:", 180, 180, 200, 1);
            ty += 16;
            if (e->bootable) {
                draw_str(g, tx + 8, ty, "Bootable [YES]", 100, 255, 100, 1);
                ty += 24;
                draw_str(g, tx, ty, "Action:", 180, 180, 200, 1);
                ty += 16;
                draw_str(g, tx + 8, ty, "Press [Enter] or [B] to boot.", 200, 200, 200, 1);
            } else {
                draw_str(g, tx + 8, ty, "Non-bootable [NO]", 255, 100, 100, 1);
                ty += 24;
                draw_str(g, tx, ty, "Action:", 180, 180, 200, 1);
                ty += 16;
                draw_str(g, tx + 8, ty, "No direct boot script.", 150, 150, 150, 1);
            }
        }
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

    if (len > 0 && len + 1 < PATH_MAX && br->cwd[len - 1] != '/') {
        br->cwd[len++] = '/';
        br->cwd[len] = '\0';
    }

    strcpy(br->cwd + len, name);

    len = strlen(br->cwd);
    if (len > 0 && len + 1 < PATH_MAX && br->cwd[len - 1] != '/') {
        br->cwd[len++] = '/';
        br->cwd[len] = '\0';
    }

    list_dir(br);
}

static int handle_boot(const char *drive, const char *path) {
    char cmd[PATH_MAX + 128];
    sprintf(cmd, "map %s%s (0xff) ;; map --hook ;; chainloader (0xff) ;; boot",
            drive, path);
    int r = run_line(cmd, BUILTIN_CMDLINE);
    if (errnum)
        return errnum;
    return r;
}

static void boot_file(const struct browser *br, struct gfx *g,
                      int px, int py, int cw, int ch) {
    const struct entry *e = &br->entries[br->cur];
    if (!e->bootable) {
        return;
    }

    char path[PATH_MAX * 2];
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

    fill_rect(g, px, py + ch - FOOTER_H * 2, cw, FOOTER_H * 2, 40, 20, 20);
    draw_str(g, px + 8, py + ch - FOOTER_H * 2 + 2, "Booting...", 255, 200, 50, 1);
    draw_str(g, px + 8, py + ch - FOOTER_H + 2, path, 200, 200, 200, 1);

    int ret = handle_boot(br->device, path);
    if (ret != 0) {
        fill_rect(g, px, py + ch - FOOTER_H * 2, cw, FOOTER_H * 2, 60, 20, 20);
        draw_str(g, px + 8, py + ch - FOOTER_H * 2 + 2, "Boot failed", 255, 50, 50, 1);
        gfx_getkey(g);
    }
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
        } else if (argv[1][0] == '/') {
            char dev[24];
            unsigned int grub_drive = current_drive >= 0x80 ? current_drive - 0x80 : current_drive;
            sprintf(dev, "(hd%u,0)", grub_drive);
            bt_drive_set_dev(dev);
            bt_drive_open_dev();
            saved_drive = current_drive;
            saved_partition = current_partition;
            strcpy(br->device, dev);
            strcpy(br->cwd, argv[1]);
        } else {
            br->cwd[0] = '\0';
        }
    } else {
        br->cwd[0] = '\0';
    }

    {
        int clen = strlen(br->cwd);
        if (clen > 0 && clen + 1 < PATH_MAX && br->cwd[clen - 1] != '/') {
            br->cwd[clen++] = '/';
            br->cwd[clen] = '\0';
        }
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
            load_selected_size(br);

            gfx_backbuffer_begin(&g);
            draw(br, &g, pad_x, pad_y, canvas_w, canvas_h);
            gfx_backbuffer_end(&g);

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
            } else if (ascii == 0x0D) {
                if (br->cur < br->count && br->entries[br->cur].bootable)
                    boot_file(br, &g, pad_x, pad_y, canvas_w, canvas_h);
                else if (br->cur < br->count) {
                    struct entry *e = &br->entries[br->cur];
                    if (e->is_dir) {
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
            } else if (scan == 0x4D) {
                if (br->cur < br->count) {
                    struct entry *e = &br->entries[br->cur];
                    if (e->is_dir) {
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
            } else if (ascii == 'b' || ascii == 'B') {
                if (br->cur < br->count && br->entries[br->cur].bootable)
                    boot_file(br, &g, pad_x, pad_y, canvas_w, canvas_h);
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
