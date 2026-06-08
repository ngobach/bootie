#ifndef BOOTIE_GFX_H
#define BOOTIE_GFX_H

#if defined(__i386__)
  #include <bios/gfx.h>
#else
  #include <uefi/gfx.h>
#endif

/* Draw a 16x16 monochrome bitmap icon.
   icon: 16 uint16_t values, bit 0 = leftmost pixel, 1 = opaque.
   Opaque pixels are drawn in the given color. */
static inline void gfx_draw_icon_16(struct gfx *ctx, int x, int y,
                                 const uint16_t *icon,
                                 uint8_t r, uint8_t g, uint8_t b) {
    for (int row = 0; row < 16; row++) {
        uint16_t bits = icon[row];
        if (!bits) continue;
        int col = 0;
        while (bits) {
            if (bits & 1) {
                int start = col;
                while (bits & 1) {
                    col++;
                    bits >>= 1;
                }
                fill_rect(ctx, x + start, y + row, col - start, 1, r, g, b);
            } else {
                col++;
                bits >>= 1;
            }
        }
    }
}

#endif /* BOOTIE_GFX_H */
