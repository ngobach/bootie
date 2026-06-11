#ifndef BOOTIE_IO_H
#define BOOTIE_IO_H

/* ========================================================================
 * Generic Drive & File API – backed by GRUB4DOS System Functions
 *
 * Thin static-inline wrappers around the raw GRUB4DOS function-pointer
 * table entries.  These eliminate the need for user code to worry about
 * calling-convention vs. direct-memory differences between BIOS and UEFI.
 * ======================================================================== */

/* ------------------------------------------------------------------ */
/*  Drive Management                                                   */
/* ------------------------------------------------------------------ */

static inline unsigned int  bt_drive_cur(void)          { return current_drive; }
static inline void          bt_drive_set_cur(unsigned int d)  { current_drive = d; }
static inline unsigned int  bt_drive_part(void)         { return current_partition; }
static inline void          bt_drive_set_part(unsigned int p) { current_partition = p; }
static inline unsigned long long bt_drive_boot(void)   { return boot_drive; }
static inline unsigned long long bt_drive_inst_part(void) { return install_partition; }
static inline unsigned long long bt_drive_part_start(void) { return part_start; }
static inline unsigned long long bt_drive_part_len(void)   { return part_length; }

/* ------------------------------------------------------------------ */
/*  Partition / Device enumeration                                     */
/* ------------------------------------------------------------------ */

static inline int    bt_drive_next_part(void)            { return next_partition(); }
static inline int    bt_drive_open_dev(void)             { return open_device(); }
static inline int    bt_drive_open_part(int flags)       { return real_open_partition(flags); }
static inline char * bt_drive_set_dev(char *s)           { return set_device(s); }

/* ------------------------------------------------------------------ */
/*  Device I/O – operates on the *current* device                      */
/* ------------------------------------------------------------------ */

static inline int bt_drive_read(unsigned long long sector,
                                unsigned long long offset,
                                unsigned long long len,
                                void *buf)
{
    return devread(sector, offset, len, (uintptr_t)buf, GRUB_READ);
}

static inline int bt_drive_write(unsigned long long sector,
                                 unsigned long long len,
                                 const void *buf)
{
    return devwrite(sector, len, (uintptr_t)buf);
}

/* ------------------------------------------------------------------ */
/*  Raw disk I/O – specifies drive explicitly                          */
/* ------------------------------------------------------------------ */

static inline int bt_drive_raw_read(unsigned int drive,
                                    unsigned long long sector,
                                    unsigned long long len,
                                    void *buf)
{
    return rawread(drive, sector, 0, len, (uintptr_t)buf, GRUB_READ);
}

static inline int bt_drive_raw_write(unsigned int drive,
                                     unsigned long long sector,
                                     const void *buf)
{
    return rawwrite(drive, (unsigned long)sector, (char *)buf);
}

static inline int bt_file_open(const char *path)
{
    return open((char *)path);
}

static inline unsigned long long bt_file_read(void *buf, unsigned long long len)
{
    return read((uintptr_t)buf, len, GRUB_READ);
}

static inline void bt_file_close(void)
{
    close();
}

static inline int bt_is_dir(const char *path) {
    if (bt_file_open(path) == 0) {
        bt_file_close();
        return 1;
    }
    bt_file_close();
    return 0;
}

static inline int bt_is_file(const char *path) {
    return !bt_is_dir(path);
}

static inline unsigned long long bt_file_size(void)  { return filesize; }

static inline unsigned long long bt_file_size_at_path(const char *path)
{
    char cmd_arg[512];
    sprintf(cmd_arg, "--length=0 %s", path);

#if defined(__i386__)
    volatile unsigned long long *p_filesize = (volatile unsigned long long *)0x8290;
#else
    volatile unsigned long long *p_filesize = (volatile unsigned long long *)IMG(0x8290);
#endif

    /* Use the official GRUB4DOS null-sink idiom: any value in (0, 0x800]
     * causes _putchar() to return early without writing anything.
     * (char_io.c: `if ((unsigned int)putchar_hooked > 0x800) *putchar_hooked++ = c`)
     * This is how GRUB4DOS itself suppresses screen output (stage2.c:2667). */
    uintptr_t _saved_ph = putchar_hooked;
    putchar_hooked = 1;   /* suppress: nonzero but ≤ 0x800 → no write */

    *p_filesize = 0;
    builtin_cmd("cat", cmd_arg, BUILTIN_CMDLINE);

    putchar_hooked = _saved_ph;
    return *p_filesize;
}
static inline unsigned long long bt_file_pos(void)   { return filepos; }
static inline void bt_file_set_pos(unsigned long long p) { filepos = p; }
static inline unsigned long long bt_file_max(void)   { return filemax; }

/* ------------------------------------------------------------------ */
/*  Directory & Filesystem                                             */
/* ------------------------------------------------------------------ */

static inline int bt_dir_list(const char *path)
{
    return grub_dir((char *)path);
}

static inline int bt_fs_type(void)
{
    return filesystem_type;
}

/* ------------------------------------------------------------------ */
/*  High-level directory listing                                       */
/* ------------------------------------------------------------------ */

struct bt_dir_entry {
    char *name;
    int is_dir;
};

static inline struct bt_dir_entry *bt_directory_list(const char *path) {
    char *buf = (char *)malloc(16384);
    if (!buf) return NULL;

    errnum = ERR_NONE;
    uintptr_t _saved = putchar_hooked;
    putchar_hooked = (uintptr_t)buf;

    bt_dir_list(path);

    char *_end = (char *)putchar_hooked;
    putchar_hooked = _saved;

    if (errnum != ERR_NONE) {
        free(buf);
        return NULL;
    }

    if (_end >= buf && _end < buf + 16384)
        *_end = '\0';
    else
        buf[16383] = '\0';

    int _cap = 64, _count = 0;
    struct bt_dir_entry *result = (struct bt_dir_entry *)malloc(
        (unsigned int)_cap * sizeof(struct bt_dir_entry));
    if (!result) { free(buf); return NULL; }

    int _plen = strlen(path);
    char *_full = (char *)malloc((unsigned int)_plen + 512);
    if (!_full) { free(buf); free(result); return NULL; }
    memmove(_full, path, (unsigned int)_plen + 1);
    if (_plen > 0 && _full[_plen - 1] != '/')
        _full[_plen++] = '/';

    char *_p = buf;
    while (*_p) {
        while (*_p == ' ') _p++;
        if (!*_p) break;

        char _name[128];
        int _n = 0;
        while (*_p && *_p != ' ') {
            if (*_p == '\\') { _p++; if (*_p) _name[_n++] = *_p++; }
            else               _name[_n++] = *_p++;
        }
        _name[_n] = '\0';

        if (_n > 0) {
            int _is_dir = 0;
            if (_name[_n - 1] == '/') {
                _name[--_n] = '\0';
                _is_dir = 1;
            }

            int _fplen = _plen;
            for (int _i = 0; _i < _n; _i++) {
                if (_name[_i] == ' ' || _name[_i] == '"' || _name[_i] == '\\')
                    _full[_fplen++] = '\\';
                _full[_fplen++] = _name[_i];
            }
            _full[_fplen] = '\0';

            if (_count + 1 >= _cap) {
                _cap *= 2;
                struct bt_dir_entry *_new = (struct bt_dir_entry *)malloc(
                    (unsigned int)_cap * sizeof(struct bt_dir_entry));
                if (!_new) break;
                memmove(_new, result, (unsigned int)_count * sizeof(struct bt_dir_entry));
                free(result);
                result = _new;
            }

            char *_entry = (char *)malloc((unsigned int)_n + 1);
            if (_entry) {
                memmove(_entry, _name, (unsigned int)_n + 1);
                result[_count].name = _entry;
                result[_count].is_dir = _is_dir || bt_is_dir(_full);
                _count++;
            }
        }
    }

    free(_full);
    result[_count].name = NULL;
    free(buf);
    return result;
}

/* ------------------------------------------------------------------ */
/*  Drive enumeration                                                  */
/* ------------------------------------------------------------------ */

#define BT_DRV_MAX   128
#define BT_PART_MAX  16

struct bt_drive_info {
    char name[24];
};

static inline int bt_device_ls(struct bt_drive_info *drives, int max) {
    int count = 0;
    unsigned long save_drv = saved_drive;
    unsigned long save_part = saved_partition;

    char *_buf = (char *)malloc(4096);
    if (!_buf) return 0;
    if (bt_eval("find", _buf, 4096) > 0) {
        char *_p = _buf;
        while (*_p && count < max) {
            while (*_p == ' ' || *_p == '\t' || *_p == '\n' || *_p == '\r')
                _p++;
            if (!*_p) break;
            char *_start = _p;
            while (*_p && *_p != ' ' && *_p != '\t' && *_p != '\n' && *_p != '\r')
                _p++;
            int _len = (int)(_p - _start);
            if (_len > 0 && _len < (int)sizeof(drives[0].name)) {
                int _i;
                for (_i = 0; _i < _len; _i++)
                    drives[count].name[_i] = _start[_i];
                drives[count].name[_len] = '\0';
                count++;
            }
        }
    }

    free(_buf);
    saved_drive = save_drv;
    saved_partition = save_part;
    return count;
}

#endif /* BOOTIE_IO_H */
