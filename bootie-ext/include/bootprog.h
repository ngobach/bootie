#ifndef BOOTPROG_H
#define BOOTPROG_H

#if defined(__i386__)
  #include <bios/grub4dos.h>
  #include <bios/grubprog.h>

  #define INIT_BOOT_MODULE() do {} while(0)
#else
  #include <uefi/grub4dos.h>
  #include <uefi/uefi.h>

  static grub_size_t g4e_data = 0;

  #include <uefi/grubprog.h>

  #define INIT_BOOT_MODULE() \
    do { \
      grub_size_t _i; \
      for (_i = 0x40100; _i <= 0x9f100; _i += 0x1000) \
      { \
        if (*(unsigned long long *)_i == 0x4946453442555247LL) \
        { \
          g4e_data = *(grub_size_t *)(_i + 16); \
          break; \
        } \
      } \
      if (!g4e_data) \
        return 0; \
    } while(0)
#endif

#endif /* BOOTPROG_H */
