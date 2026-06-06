#ifndef BOOTPROG_H
#define BOOTPROG_H

#if defined(__i386__)
  #include <bios/grub4dos.h>
  #include <bios/grubprog.h>
#else
  #include <uefi/grub4dos.h>
  #include <uefi/uefi.h>

  static grub_size_t g4e_data = 0;

  #include <uefi/grubprog.h>
#endif

extern int gmain(char *arg, int flags);

int main(char *arg, int flags) {
  /* Zero BSS section */
  char *bss = (char *)&__BSS_START;
  char *bss_end = (char *)&__BSS_END;
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
  return gmain(arg, flags);
}

#endif /* BOOTPROG_H */
