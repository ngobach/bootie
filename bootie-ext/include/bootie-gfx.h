#ifndef BOOTIE_GFX_H
#define BOOTIE_GFX_H

/* ------------------------------------------------------------------ */
/*  Delay using PIT channel 2 (8253/8254) – BIOS target                */
/*  Works on x86 (32-bit); in/out opcodes are valid. UEFI uses         */
/*  the firmware stall() instead because PIT IO ports may be           */
/*  restricted in long mode.                                           */
/* ------------------------------------------------------------------ */
static void pit_delay_ms(unsigned int ms) {
    if (!ms) return;

    /* Enable PIT channel 2 gate (bit 0 of port 0x61 = Timer 2 gate) */
    uint8_t tmp;
    __asm__ volatile("inb %%dx, %%al" : "=a"(tmp) : "d"((uint16_t)0x61));
    __asm__ volatile("outb %%al, %%dx" : : "a"((uint8_t)(tmp | 1)), "d"((uint16_t)0x61));

    /* Break into chunks ≤ 50 ms (PIT max ≈ 54.9 ms at 1.193182 MHz) */
    while (ms) {
        unsigned int chunk = ms > 50 ? 50 : ms;
        uint32_t count = (uint32_t)chunk * 1193182U / 1000U;
        if (count < 2) count = 2;

        /* Program PIT channel 2: mode 0, LSB then MSB, binary */
        __asm__ volatile("outb %%al, %%dx" : : "a"((uint8_t)0xB0), "d"((uint16_t)0x43));
        __asm__ volatile("outb %%al, %%dx" : : "a"((uint8_t)(count)), "d"((uint16_t)0x42));
        __asm__ volatile("outb %%al, %%dx" : : "a"((uint8_t)(count >> 8)), "d"((uint16_t)0x42));

        /* Poll until counter reaches 0 */
        uint8_t lo, hi;
        do {
            __asm__ volatile("outb %%al, %%dx" : : "a"((uint8_t)0x82), "d"((uint16_t)0x43));
            __asm__ volatile("inb %%dx, %%al" : "=a"(lo) : "d"((uint16_t)0x42));
            __asm__ volatile("inb %%dx, %%al" : "=a"(hi) : "d"((uint16_t)0x42));
        } while (lo | hi);

        ms -= chunk;
    }
}

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
