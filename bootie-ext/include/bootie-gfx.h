#ifndef BOOTIE_GFX_H
#define BOOTIE_GFX_H

#if defined(__i386__)
  #include <bios/gfx.h>
#else
  #include <uefi/gfx.h>
#endif

#endif /* BOOTIE_GFX_H */
