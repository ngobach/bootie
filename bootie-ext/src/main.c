#if defined(__i386__)
  #include <bios/grub4dos.h>
  #include <bios/grubprog.h>
#else
  #include <uefi/grub4dos.h>
  #include <uefi/uefi.h>

  static grub_size_t g4e_data = 0;
  static void get_G4E_image(void);

  #include <uefi/grubprog.h>
#endif

int main(char *arg, int flags)
{
#ifndef __i386__
  get_G4E_image();
  if (!g4e_data)
    return 0;
#endif

  return printf("Hello world!\n");
}

#ifndef __i386__
static void get_G4E_image(void)
{
  grub_size_t i;
  // Search memory for "GRUB4EFI" signature to resolve dynamic base address
  for (i = 0x40100; i <= 0x9f100 ; i += 0x1000)
  {
    if (*(unsigned long long *)i == 0x4946453442555247LL)
    {
      g4e_data = *(grub_size_t *)(i+16);
      return;
    }
  }
}
#endif
