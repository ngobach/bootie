#ifndef BOOTIE_H
#define BOOTIE_H

#include <stdint.h>

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

  static grub_size_t g4e_data = 0;

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

/* Run a GRUB4DOS command and capture its output into buf (null-terminated).
 * Returns the number of bytes written (excluding terminator), or -1 on error.
 * buf may be NULL to discard output and only check errnum. */
static inline int bt_eval(const char *cmd, char *buf, int bufsize) {
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
  while (bss < bss_end) {
    *bss++ = 0;
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
