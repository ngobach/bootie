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

static inline unsigned long long bt_file_size(void)  { return filesize; }

static inline unsigned long long bt_file_get_size(const char *path)
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
/*  Drive enumeration                                                  */
/* ------------------------------------------------------------------ */

#define BT_DRV_MAX   128
#define BT_PART_MAX  16

struct bt_drive_info {
    char name[24];
};

static inline int bt_drive_enum(struct bt_drive_info *drives, int max) {
    int count = 0;
    unsigned long save_drv = saved_drive;
    unsigned long save_part = saved_partition;

    char _buf[4096];
    if (bt_eval("find", _buf, sizeof(_buf)) > 0) {
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

    saved_drive = save_drv;
    saved_partition = save_part;
    return count;
}

#endif /* BOOTIE_IO_H */
