#ifndef GFX_H
#define GFX_H

#if defined(__i386__)
  #include <bios/gfx.h>
#else
  #include <uefi/gfx.h>
#endif

#endif /* GFX_H */
