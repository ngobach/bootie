#ifndef BOOTIE_H
#define BOOTIE_H

#include <stdint.h>
#include <string.h>

#if defined(__i386__)
  #include <bios/grub4dos.h>
  /* Linker symbols defining the BSS segment boundaries.
     In 32-bit BIOS mode, they are declared as standard extern variables. Their
     addresses are retrieved using the address-of operator (&__BSS_START) in
     bootie.h. */
  extern int __BSS_END;
  extern int __BSS_START;
#else
  #include <uefi/grub4dos.h>
  #include <uefi/uefi.h>

  static grub_size_t g4e_data;

  /* Linker symbols defining the BSS segment boundaries.
     We declare them as hidden character arrays so that GCC compiles position-independent,
     RIP-relative address calculations (using 'lea') instead of absolute address loads
     or GOT dereferencing. */
  __attribute__((visibility("hidden"))) extern char __BSS_END[];
  __attribute__((visibility("hidden"))) extern char __BSS_START[];
#endif

static uint32_t rand_seed = 12345;

static inline void seed_rand(uint32_t s) {
#if defined(__i386__)
    s += *(volatile unsigned long *)0x46c;
#endif
    rand_seed = s;
}

/* Linear Congruential Generator for simple PRNG */
static inline uint32_t next_rand(void) {
    rand_seed = rand_seed * 1103515245 + 12345;
    return (rand_seed / 65536) % 32768;
}

#define BT_EVAL_F_ERRMSG 1

static inline const char *bt_errstr(grub_error_t e) {
    switch (e) {
    case ERR_BAD_ARGUMENT:    return "Invalid argument";
    case ERR_BAD_FILENAME:    return "Filename must be either an absolute pathname or blocklist";
    case ERR_BAD_FILETYPE:    return "Bad file or directory type";
    case ERR_BAD_GZIP_DATA:   return "Bad or corrupt data while decompressing file";
    case ERR_BAD_GZIP_HEADER: return "Bad or incompatible header in compressed file";
    case ERR_BAD_PART_TABLE:  return "Partition table invalid or corrupt";
    case ERR_BAD_VERSION:     return "Mismatched or corrupt version of stage1/stage2";
    case ERR_BELOW_1MB:       return "Loading below 1MB is not supported";
    case ERR_BOOT_COMMAND:    return "Kernel must be loaded before booting";
    case ERR_BOOT_FAILURE:    return "Unknown boot failure";
    case ERR_BOOT_FEATURES:   return "Unsupported Multiboot features requested";
    case ERR_DEV_FORMAT:      return "Unrecognized device string, or you omitted the required DEVICE part which should lead the filename.";
    case ERR_DEV_NEED_INIT:   return "Device not initialized yet";
    case ERR_DEV_VALUES:      return "Invalid device requested";
    case ERR_EXEC_FORMAT:     return "Invalid or unsupported executable format";
    case ERR_FILELENGTH:      return "Filesystem compatibility error, cannot read whole file";
    case ERR_FILE_NOT_FOUND:  return "File not found";
    case ERR_FSYS_CORRUPT:    return "Inconsistent filesystem structure";
    case ERR_FSYS_MOUNT:      return "Cannot mount selected partition";
    case ERR_GEOM:            return "Selected cylinder exceeds maximum supported by BIOS";
    case ERR_NEED_LX_KERNEL:  return "Linux kernel must be loaded before initrd";
    case ERR_NEED_MB_KERNEL:  return "Multiboot kernel must be loaded before modules";
    case ERR_NO_DISK:         return "Selected disk does not exist";
    case ERR_NO_DISK_SPACE:   return "No spare sectors on the disk";
    case ERR_NO_PART:         return "No such partition";
    case ERR_NUMBER_OVERFLOW: return "Overflow while parsing number";
    case ERR_NUMBER_PARSING:  return "Error while parsing number";
    case ERR_OUTSIDE_PART:    return "Attempt to access block outside partition";
    case ERR_PRIVILEGED:      return "Must be authenticated";
    case ERR_READ:            return "Disk read error";
    case ERR_SYMLINK_LOOP:    return "Too many symbolic links";
    case ERR_UNALIGNED:       return "File is not sector aligned";
    case ERR_UNRECOGNIZED:    return "Unrecognized command";
    case ERR_WONT_FIT:        return "Selected item cannot fit into memory";
    case ERR_WRITE:           return "Disk write error";
    default:                  return NULL;
    }
}

/* Run a GRUB4DOS command and capture its output into buf (null-terminated).
 * Returns the number of bytes written (excluding terminator), or -1 on error.
 * buf may be NULL to discard output and only check errnum.
 * When (flags & BT_EVAL_F_ERRMSG) and errnum is set after the command,
 * the error description is appended to buf. */
static inline int bt_eval_ex(const char *cmd, char *buf, int bufsize, int flags) {
    if (buf && bufsize < 1) return -1;
    errnum = ERR_NONE;
    uintptr_t saved = putchar_hooked;
    if (buf)
        putchar_hooked = (uintptr_t)buf;
    else
        putchar_hooked = 1; /* non-zero but ≤ 0x800 → discard (GRUB4DOS idiom) */
    run_line((char *)cmd, BUILTIN_CMDLINE);
    char *end = (char *)putchar_hooked;
    putchar_hooked = saved;

    if (errnum != ERR_NONE && (flags & BT_EVAL_F_ERRMSG) && buf) {
        char *wp = buf;
        int rem = bufsize;
        if (end > buf && end < buf + bufsize) {
            wp = end;
            rem = (int)(buf + bufsize - end);
        }
        if (rem > 10) {
            const char *estr = bt_errstr(errnum);
            if (!estr) estr = "Unknown error";
            char eb[192];
            int n = sprintf(eb, "\nError %d: %s", (int)errnum, estr);
            if (n > rem) n = rem;
            memcpy(wp, eb, n);
            end = wp + n;
            *end = '\0';
        }
    }

    if (errnum != ERR_NONE) return -1;
    if (buf) {
        int count;
        if (end > buf && end < buf + bufsize) {
            *end = '\0';
            count = (int)(end - buf);
        } else {
            buf[bufsize - 1] = '\0';
            count = bufsize - 1;
        }
        return count;
    }
    return 0;
}

/* Forward to bt_eval_ex with no flags (backward-compatible). */
static inline int bt_eval(const char *cmd, char *buf, int bufsize) {
    return bt_eval_ex(cmd, buf, bufsize, 0);
}

extern int gmain(int argc, char *argv[], int flags);

int main(char *arg, int flags) {
  /* Zero BSS section */
#if defined(__i386__)
  char *bss = (char *)&__BSS_START;
  char *bss_end = (char *)&__BSS_END;
#else
  char *bss = __BSS_START;
  char *bss_end = __BSS_END;
#endif
  {
    unsigned long len = (unsigned long)(bss_end - bss);
    unsigned char *p = bss;
    unsigned long n = len >> 2;
    if (n)
        __asm__ __volatile__("rep stosl" : "+D"(p), "+c"(n) : "a"(0) : "memory");
    n = len & 3;
    if (n)
        __asm__ __volatile__("rep stosb" : "+D"(p), "+c"(n) : "a"(0) : "memory");
  }

#if !defined(__i386__)
  grub_size_t _i;
  for (_i = 0x40100; _i <= 0x9f100; _i += 0x1000) {
    if (*(unsigned long long *)_i == 0x4946453442555247LL) {
      g4e_data = *(grub_size_t *)(_i + 16);
      break;
    }
  }
  if (!g4e_data) {
    return 0;
  }
#endif

  /* Auto-seed the PRNG at startup */
  uint32_t seed = 12345;
#if defined(__i386__)
  seed += *(volatile unsigned long *)0x46c;
#else
  efi_system_table_t *st = grub_efi_system_table;
  if (st && st->runtime_services) {
    efi_time_t et;
    if (st->runtime_services->get_time(&et, NULL) == 0) {
      seed += et.second + et.minute * 60 + et.hour * 3600 + et.nanosecond;
    }
  }
#endif
  seed_rand(seed);

  char *argv[16];
  int argc = 0;
  argv[argc++] = "bootprog";
  char *p = arg;
  if (!p) { argv[argc] = 0; return gmain(argc, argv, flags); }
  while (*p) {
      while (*p == ' ') p++;
      if (!*p) break;
      if (*p == '"') {
          p++;
          argv[argc++] = p;
          while (*p && *p != '"') p++;
          if (*p) *p++ = '\0';
      } else {
          argv[argc++] = p;
          while (*p && *p != ' ') p++;
          if (*p) *p++ = '\0';
      }
      if (argc >= 15) break;
  }
  argv[argc] = 0;

  return gmain(argc, argv, flags);
}

#endif /* BOOTIE_H */
