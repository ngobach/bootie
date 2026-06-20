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
/*  Text rendering (RGBA sprites)                                      */
/* ------------------------------------------------------------------ */

static inline void gfx_sprite_draw_str(struct gfx_sprite *dst, struct gfx *ctx,
                                        int x, int y, const char *str,
                                        uint8_t r, uint8_t g, uint8_t b,
                                        uint8_t a, int px_size) {
    if (!str || !str[0]) return;

    /* Build a fake gfx pointing to the sprite buffer in RGBA format.
       The font renderer writes using the ctx's rshift/gshift/bshift
       and now handles alpha compositing correctly. */
    struct gfx fake;
    fake.fb = dst->pixels;
    fake.width = dst->w;
    fake.height = dst->h;
    fake.pitch = dst->w * 4;
    fake.bpp = 4;
    fake.rshift = 0;
    fake.gshift = 8;
    fake.bshift = 16;
    draw_str(&fake, x, y, str, r, g, b, px_size);
}

#endif /* BOOTIE_SPRITE_H */
