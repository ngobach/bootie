#ifndef BOOTIE_GFX_H
#define BOOTIE_GFX_H

#if defined(__i386__)
  #include <bios/gfx.h>
#else
  #include <uefi/gfx.h>
#endif

struct gfx_sprite {
    unsigned char *pixels;      /* RGBA pixel data (memory) or native format (screen) */
    unsigned w, h;              /* dimensions */
    uint8_t is_screen;          /* 1 = wraps hardware framebuffer (native pixel format) */
    /* Screen sprite fields (set by gfx_sprite_from_fb): */
    uint32_t pitch;             /* bytes per scanline */
    uint8_t rshift, gshift, bshift, bpp;
};

/* Include TTF font rendering (overrides draw_str with a TTF-aware macro
   when a font is loaded via gfx_font_load()). */
#include <bootie-font.h>

/* Include sprite drawing API. */
#if __has_include("bootie-sprite.h")
#  include <bootie-sprite.h>
#endif

/* Include PNG image loader (lodepng-based, memory-only decode).
   Provides gfx_png_decode(), gfx_png_free(). */
#if __has_include("bootie-img.h")
#  include <bootie-img.h>
#endif

#endif /* BOOTIE_GFX_H */
