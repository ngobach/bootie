#ifndef GFX_H
#define GFX_H

#if defined(__i386__)
  #include <bios/gfx.h>
#else
  // UEFI gfx.h will be included here once implemented
#endif

#endif /* GFX_H */
