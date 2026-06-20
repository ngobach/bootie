#ifndef BOOTIE_SPRITE_H
#define BOOTIE_SPRITE_H

#include <bootie-gfx.h>

/* ------------------------------------------------------------------ */
/*  Lifecycle                                                          */
/* ------------------------------------------------------------------ */

/* Initialize a memory sprite with an RGBA pixel buffer. */
static inline void gfx_sprite_init(struct gfx_sprite *s, unsigned w, unsigned h) {
    s->pixels = (unsigned char *)zalloc(w * h * 4);
    s->w = w;
    s->h = h;
}

/* Free pixel buffer. */
static inline void gfx_sprite_destroy(struct gfx_sprite *s) {
    if (s->pixels) {
        free(s->pixels);
    }
    s->pixels = 0;
}

/* ------------------------------------------------------------------ */
/*  Sprite fill (RGBA)                                                 */
/* ------------------------------------------------------------------ */

static inline void gfx_sprite_fill_rect(struct gfx_sprite *dst,
                                         int x, int y, int w, int h,
                                         uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (w <= 0 || h <= 0) return;
    if ((unsigned)x >= dst->w || (unsigned)y >= dst->h) return;
    if ((unsigned)(x + w) > dst->w) w = dst->w - x;
    if ((unsigned)(y + h) > dst->h) h = dst->h - y;

    if (a == 255) {
        uint32_t color = (uint32_t)r | ((uint32_t)g << 8) |
                         ((uint32_t)b << 16) | ((uint32_t)255 << 24);
        for (int row = 0; row < h; row++) {
            uint32_t *p = (uint32_t *)(dst->pixels + ((unsigned)(y + row) * dst->w + (unsigned)x) * 4);
            uint32_t n = (uint32_t)w;
            __asm__ __volatile__("rep stosl" : "+D"(p), "+c"(n) : "a"(color) : "memory");
        }
    } else {
        for (int row = 0; row < h; row++) {
            unsigned char *p = dst->pixels + ((unsigned)(y + row) * dst->w + (unsigned)x) * 4;
            for (int col = 0; col < w; col++) {
                int inv = 255 - a;
                p[0] = (uint8_t)((r * a + p[0] * inv + 128) >> 8);
                p[1] = (uint8_t)((g * a + p[1] * inv + 128) >> 8);
                p[2] = (uint8_t)((b * a + p[2] * inv + 128) >> 8);
                p[3] = (uint8_t)((255 * a + p[3] * inv + 128) >> 8);
                p += 4;
            }
        }
    }
}

static inline void gfx_sprite_put_pixel(struct gfx_sprite *dst,
                                         int x, int y,
                                         uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if ((unsigned)x >= dst->w || (unsigned)y >= dst->h) return;
    unsigned char *p = dst->pixels + ((unsigned)y * dst->w + (unsigned)x) * 4;
    p[0] = r; p[1] = g; p[2] = b; p[3] = a;
}

static inline void gfx_sprite_fill(struct gfx_sprite *dst,
                                    int x, int y, int w, int h,
                                    uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (w <= 0 || h <= 0) return;
    if ((unsigned)x >= dst->w || (unsigned)y >= dst->h) return;
    if ((unsigned)(x + w) > dst->w) w = dst->w - x;
    if ((unsigned)(y + h) > dst->h) h = dst->h - y;

    if (a == 255) {
        uint32_t color = (uint32_t)r | ((uint32_t)g << 8) |
                         ((uint32_t)b << 16) | ((uint32_t)255 << 24);
        for (int row = 0; row < h; row++) {
            uint32_t *p = (uint32_t *)(dst->pixels + ((unsigned)(y + row) * dst->w + (unsigned)x) * 4);
            uint32_t n = (uint32_t)w;
            __asm__ __volatile__("rep stosl" : "+D"(p), "+c"(n) : "a"(color) : "memory");
        }
    } else {
        for (int row = 0; row < h; row++) {
            unsigned char *p = dst->pixels + ((unsigned)(y + row) * dst->w + (unsigned)x) * 4;
            for (int col = 0; col < w; col++) {
                int inv = 255 - a;
                p[0] = (uint8_t)((r * a + p[0] * inv + 128) >> 8);
                p[1] = (uint8_t)((g * a + p[1] * inv + 128) >> 8);
                p[2] = (uint8_t)((b * a + p[2] * inv + 128) >> 8);
                p[3] = (uint8_t)((255 * a + p[3] * inv + 128) >> 8);
                p += 4;
            }
        }
    }
}

static inline void gfx_sprite_clear(struct gfx_sprite *dst,
                                     uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    gfx_sprite_fill(dst, 0, 0, (int)dst->w, (int)dst->h, r, g, b, a);
}

/* ------------------------------------------------------------------ */
/*  Sprite-to-sprite blit (RGBA → RGBA, alpha-composited)              */
/* ------------------------------------------------------------------ */

static inline void gfx_sprite_blit(struct gfx_sprite *dst,
                                    const struct gfx_sprite *src,
                                    int dx, int dy) {
    if (!src->pixels || !dst->pixels) return;
    int sx = 0, sy = 0;
    int sw = (int)src->w, sh = (int)src->h;

    if (dx < 0) { sx = -dx; sw += dx; dx = 0; }
    if (dy < 0) { sy = -dy; sh += dy; dy = 0; }
    if ((unsigned)dx >= dst->w || (unsigned)dy >= dst->h) return;
    if (dx + sw > (int)dst->w) sw = dst->w - dx;
    if (dy + sh > (int)dst->h) sh = dst->h - dy;
    if (sw <= 0 || sh <= 0) return;

    for (int row = 0; row < sh; row++) {
        const unsigned char *sp = src->pixels + ((unsigned)(sy + row) * src->w + (unsigned)sx) * 4;
        unsigned char *dp = dst->pixels + ((unsigned)(dy + row) * dst->w + (unsigned)dx) * 4;
        for (int col = 0; col < sw; col++) {
            uint8_t sa = sp[3];
            if (sa == 255) {
                dp[0] = sp[0]; dp[1] = sp[1]; dp[2] = sp[2]; dp[3] = 255;
            } else if (sa > 0) {
                int inv = 255 - sa;
                dp[0] = (uint8_t)((sp[0] * sa + dp[0] * inv + 128) >> 8);
                dp[1] = (uint8_t)((sp[1] * sa + dp[1] * inv + 128) >> 8);
                dp[2] = (uint8_t)((sp[2] * sa + dp[2] * inv + 128) >> 8);
                dp[3] = (uint8_t)((255 * sa + dp[3] * inv + 128) >> 8);
            }
            sp += 4; dp += 4;
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Draw sprite to gfx canvas (RGBA → native, alpha-composited)        */
/*  This is the ONLY operation that reads sprite pixels and writes     */
/*  to the native-format canvas.                                       */
/* ------------------------------------------------------------------ */

static inline void gfx_draw_sprite(struct gfx *g, const struct gfx_sprite *src,
                                    int dx, int dy) {
    if (!src->pixels || !g->fb) return;
    int sx = 0, sy = 0;
    int sw = (int)src->w, sh = (int)src->h;

    if (dx < 0) { sx = -dx; sw += dx; dx = 0; }
    if (dy < 0) { sy = -dy; sh += dy; dy = 0; }
    if ((unsigned)dx >= g->width || (unsigned)dy >= g->height) return;
    if (dx + sw > (int)g->width)  sw = g->width - dx;
    if (dy + sh > (int)g->height) sh = g->height - dy;
    if (sw <= 0 || sh <= 0) return;

    for (int row = 0; row < sh; row++) {
        const unsigned char *sp = src->pixels + ((unsigned)(sy + row) * src->w + (unsigned)sx) * 4;
        unsigned char *dp = g->fb + (unsigned)(dy + row) * g->pitch + (unsigned)dx * g->bpp;

        for (int col = 0; col < sw; col++) {
            uint8_t sr = sp[0], sg = sp[1], sb = sp[2], sa = sp[3];

            if (sa == 255) {
                /* Opaque: write native directly */
                uint32_t color = ((uint32_t)sr << g->rshift) |
                                 ((uint32_t)sg << g->gshift) |
                                 ((uint32_t)sb << g->bshift);
                if (g->bpp == 4) {
                    *(uint32_t *)dp = color;
                } else if (g->bpp == 3) {
                    dp[0] = color & 0xFF;
                    dp[1] = (color >> 8) & 0xFF;
                    dp[2] = (color >> 16) & 0xFF;
                } else if (g->bpp == 2) {
                    *(uint16_t *)dp = (uint16_t)(((sr >> 3) << 11) | ((sg >> 2) << 5) | (sb >> 3));
                }
            } else if (sa > 0) {
                /* Alpha composite: read native dst, blend, write native */
                uint32_t dst_color = 0;
                uint8_t dr, dg, db;
                if (g->bpp == 4) {
                    dst_color = *(uint32_t *)dp;
                } else if (g->bpp == 3) {
                    dst_color = dp[0] | ((uint32_t)dp[1] << 8) | ((uint32_t)dp[2] << 16);
                } else if (g->bpp == 2) {
                    dst_color = *(uint16_t *)dp;
                    /* Expand 5-6-5 to 8-bit for blending */
                    dr = (uint8_t)(((dst_color >> 11) & 0x1F) << 3);
                    dg = (uint8_t)(((dst_color >> 5)  & 0x3F) << 2);
                    db = (uint8_t)((dst_color & 0x1F) << 3);
                    int inv = 255 - sa;
                    uint8_t fr = (uint8_t)((sr * sa + dr * inv + 128) >> 8);
                    uint8_t fg = (uint8_t)((sg * sa + dg * inv + 128) >> 8);
                    uint8_t fb_ = (uint8_t)((sb * sa + db * inv + 128) >> 8);
                    *(uint16_t *)dp = (uint16_t)(((fr >> 3) << 11) | ((fg >> 2) << 5) | (fb_ >> 3));
                    sp += 4; dp += g->bpp;
                    continue;
                }
                dr = (uint8_t)((dst_color >> g->rshift) & 0xFF);
                dg = (uint8_t)((dst_color >> g->gshift) & 0xFF);
                db = (uint8_t)((dst_color >> g->bshift) & 0xFF);
                int inv = 255 - sa;
                uint8_t fr = (uint8_t)((sr * sa + dr * inv + 128) >> 8);
                uint8_t fg = (uint8_t)((sg * sa + dg * inv + 128) >> 8);
                uint8_t fb_ = (uint8_t)((sb * sa + db * inv + 128) >> 8);
                uint32_t result = ((uint32_t)fr << g->rshift) |
                                  ((uint32_t)fg << g->gshift) |
                                  ((uint32_t)fb_ << g->bshift);
                if (g->bpp == 4) {
                    *(uint32_t *)dp = result;
                } else if (g->bpp == 3) {
                    dp[0] = result & 0xFF;
                    dp[1] = (result >> 8) & 0xFF;
                    dp[2] = (result >> 16) & 0xFF;
                }
            }
            sp += 4;
            dp += g->bpp;
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Flush RGBA sprite directly to hardware screen                      */
/*  Converts RGBA → native and pushes to screen.                       */
/* ------------------------------------------------------------------ */
static inline void gfx_flush_sprite(struct gfx *g, const struct gfx_sprite *spr) {
    if (!spr->pixels) return;

    uint32_t w = g->width < spr->w ? g->width : spr->w;
    uint32_t h = g->height < spr->h ? g->height : spr->h;

#if defined(__i386__)
    /*
     * BIOS single-pass: RGBA sprite → native pixels directly into hw_fb.
     *
     * The intermediate canvas (g->fb) sits in uncacheable memory under
     * GRUB4DOS, making the old two-pass approach (write canvas + copy to
     * VRAM) ~140× slower than UEFI.  By writing directly to the hardware
     * framebuffer we cut the UC traffic in half.
     *
     * Alpha handling: the sprite is pre-composited in RGBA space, so
     * all visible pixels should be fully opaque (a==255).  We skip
     * transparent pixels (a==0) and write semi-transparent ones as
     * opaque — reading the VRAM destination for blending would be
     * extremely slow on UC memory.
     */
    if (!g->hw_fb) return;
    int ox = ((int)g->hw_width  - (int)g->width)  / 2;
    int oy = ((int)g->hw_height - (int)g->height) / 2;
    if (ox < 0) ox = 0;
    if (oy < 0) oy = 0;

    for (uint32_t row = 0; row < h; row++) {
        const uint8_t *sp = spr->pixels + row * spr->w * 4;
        uint8_t *dp = g->hw_fb + (unsigned)(oy + (int)row) * g->hw_pitch
                     + (unsigned)ox * g->bpp;

        if (g->bpp == 4) {
            uint32_t *dst = (uint32_t *)dp;
            for (uint32_t col = 0; col < w; col++) {
                uint8_t sr = sp[0], sg = sp[1], sb = sp[2], sa = sp[3];
                if (sa > 0) {
                    dst[col] = ((uint32_t)sr << g->rshift) |
                               ((uint32_t)sg << g->gshift) |
                               ((uint32_t)sb << g->bshift);
                }
                sp += 4;
            }
        } else if (g->bpp == 3) {
            for (uint32_t col = 0; col < w; col++) {
                uint8_t sr = sp[0], sg = sp[1], sb = sp[2], sa = sp[3];
                if (sa > 0) {
                    uint32_t color = ((uint32_t)sr << g->rshift) |
                                     ((uint32_t)sg << g->gshift) |
                                     ((uint32_t)sb << g->bshift);
                    dp[col*3+0] = color & 0xFF;
                    dp[col*3+1] = (color >> 8) & 0xFF;
                    dp[col*3+2] = (color >> 16) & 0xFF;
                }
                sp += 4;
            }
        } else if (g->bpp == 2) {
            uint16_t *dst16 = (uint16_t *)dp;
            for (uint32_t col = 0; col < w; col++) {
                uint8_t sr = sp[0], sg = sp[1], sb = sp[2], sa = sp[3];
                if (sa > 0) {
                    dst16[col] = (uint16_t)(((sr >> 3) << 11) | ((sg >> 2) << 5) | (sb >> 3));
                }
                sp += 4;
            }
        }
    }
#else
    /* UEFI: convert RGBA → native in canvas, then Blt to screen */
    if (!g->fb) return;

    for (uint32_t row = 0; row < h; row++) {
        const uint8_t *sp = spr->pixels + row * spr->w * 4;
        uint8_t *dp = g->fb + row * g->pitch;

        if (g->bpp == 4) {
            uint32_t *dst = (uint32_t *)dp;
            for (uint32_t col = 0; col < w; col++) {
                uint8_t sr = sp[0], sg = sp[1], sb = sp[2], sa = sp[3];
                if (sa == 255) {
                    dst[col] = ((uint32_t)sr << g->rshift) |
                               ((uint32_t)sg << g->gshift) |
                               ((uint32_t)sb << g->bshift);
                } else if (sa > 0) {
                    uint32_t dc = dst[col];
                    uint8_t dr = (uint8_t)((dc >> g->rshift) & 0xFF);
                    uint8_t dg = (uint8_t)((dc >> g->gshift) & 0xFF);
                    uint8_t db = (uint8_t)((dc >> g->bshift) & 0xFF);
                    int inv = 255 - sa;
                    uint8_t fr = (uint8_t)((sr * sa + dr * inv + 128) >> 8);
                    uint8_t fg = (uint8_t)((sg * sa + dg * inv + 128) >> 8);
                    uint8_t fb_ = (uint8_t)((sb * sa + db * inv + 128) >> 8);
                    dst[col] = ((uint32_t)fr << g->rshift) |
                               ((uint32_t)fg << g->gshift) |
                               ((uint32_t)fb_ << g->bshift);
                }
                sp += 4;
            }
        } else if (g->bpp == 3) {
            for (uint32_t col = 0; col < w; col++) {
                uint8_t sr = sp[0], sg = sp[1], sb = sp[2], sa = sp[3];
                if (sa == 255) {
                    uint32_t color = ((uint32_t)sr << g->rshift) |
                                     ((uint32_t)sg << g->gshift) |
                                     ((uint32_t)sb << g->bshift);
                    dp[col*3+0] = color & 0xFF;
                    dp[col*3+1] = (color >> 8) & 0xFF;
                    dp[col*3+2] = (color >> 16) & 0xFF;
                } else if (sa > 0) {
                    uint32_t dc = dp[col*3] | ((uint32_t)dp[col*3+1] << 8) | ((uint32_t)dp[col*3+2] << 16);
                    uint8_t dr = (uint8_t)((dc >> g->rshift) & 0xFF);
                    uint8_t dg = (uint8_t)((dc >> g->gshift) & 0xFF);
                    uint8_t db = (uint8_t)((dc >> g->bshift) & 0xFF);
                    int inv = 255 - sa;
                    uint8_t fr = (uint8_t)((sr * sa + dr * inv + 128) >> 8);
                    uint8_t fg = (uint8_t)((sg * sa + dg * inv + 128) >> 8);
                    uint8_t fb_ = (uint8_t)((sb * sa + db * inv + 128) >> 8);
                    uint32_t result = ((uint32_t)fr << g->rshift) |
                                      ((uint32_t)fg << g->gshift) |
                                      ((uint32_t)fb_ << g->bshift);
                    dp[col*3+0] = result & 0xFF;
                    dp[col*3+1] = (result >> 8) & 0xFF;
                    dp[col*3+2] = (result >> 16) & 0xFF;
                }
                sp += 4;
            }
        } else if (g->bpp == 2) {
            uint16_t *dst16 = (uint16_t *)dp;
            for (uint32_t col = 0; col < w; col++) {
                uint8_t sr = sp[0], sg = sp[1], sb = sp[2], sa = sp[3];
                if (sa == 255) {
                    dst16[col] = (uint16_t)(((sr >> 3) << 11) | ((sg >> 2) << 5) | (sb >> 3));
                } else if (sa > 0) {
                    uint16_t dc = dst16[col];
                    uint8_t dr = (uint8_t)(((dc >> 11) & 0x1F) << 3);
                    uint8_t dg = (uint8_t)(((dc >> 5) & 0x3F) << 2);
                    uint8_t db = (uint8_t)((dc & 0x1F) << 3);
                    int inv = 255 - sa;
                    uint8_t fr = (uint8_t)((sr * sa + dr * inv + 128) >> 8);
                    uint8_t fg = (uint8_t)((sg * sa + dg * inv + 128) >> 8);
                    uint8_t fb_ = (uint8_t)((sb * sa + db * inv + 128) >> 8);
                    dst16[col] = (uint16_t)(((fr >> 3) << 11) | ((fg >> 2) << 5) | (fb_ >> 3));
                }
                sp += 4;
            }
        }
    }

    /* UEFI: Blt canvas to screen, centered */
    if (!g->gop) return;
    efi_graphics_output_protocol_t *gop =
        (efi_graphics_output_protocol_t *)g->gop;
    if (!gop->Blt) return;
    typedef efi_status_t(EFIAPI *blt_t)(void *, void *, uint32_t,
                                         uint32_t, uint32_t, uint32_t,
                                         uint32_t, uint32_t, uint32_t,
                                         uint32_t);
    int ox = ((int)g->hw_width  - (int)g->width)  / 2;
    int oy = ((int)g->hw_height - (int)g->height) / 2;
    if (ox < 0) ox = 0;
    if (oy < 0) oy = 0;
    ((blt_t)gop->Blt)(gop, g->fb, 2 /*EfiBltBufferToVideo*/,
                       0, 0, (uint32_t)ox, (uint32_t)oy,
                       g->width, g->height, g->pitch);
#endif
}

/* ------------------------------------------------------------------ */
/*  Text rendering (RGBA sprites)                                      */
/* ------------------------------------------------------------------ */

static inline void gfx_sprite_draw_str(struct gfx_sprite *dst,
                                        int x, int y, const char *str,
                                        uint8_t r, uint8_t g, uint8_t b,
                                        uint8_t a, int px_size) {
    if (!str || !str[0]) return;
    draw_str(dst, x, y, str, r, g, b, px_size);
}

#endif /* BOOTIE_SPRITE_H */
