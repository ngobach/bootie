#ifndef BOOTIE_GFX_H
#define BOOTIE_GFX_H

#if defined(__i386__)
  #include <bios/gfx.h>
#else
  #include <uefi/gfx.h>
#endif

struct gfx_image {
    unsigned char *pixels;
    unsigned w, h;
};

/* Include TTF font rendering (overrides draw_str with a TTF-aware macro
   when a font is loaded via gfx_font_load()). */
#include <bootie-font.h>

/* Include PNG image loader (lodepng-based, memory-only decode).
   Provides gfx_png_decode(), gfx_png_free(), gfx_draw_image(). */
#if __has_include("bootie-img.h")
#  include <bootie-img.h>
#endif

#endif /* BOOTIE_GFX_H */
