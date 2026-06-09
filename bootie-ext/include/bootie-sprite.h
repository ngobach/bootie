#ifndef BOOTIE_SPRITE_H
#define BOOTIE_SPRITE_H

#include <bootie-gfx.h>

/* ------------------------------------------------------------------ */
/*  Lifecycle                                                          */
/* ------------------------------------------------------------------ */

/* Initialize a memory sprite with an RGBA pixel buffer. */
static inline void gfx_sprite_init(struct gfx_sprite *s, unsigned w, unsigned h) {
    s->pixels = (unsigned char *)malloc(w * h * 4);
    if (s->pixels) {
        unsigned n = w * h * 4;
        for (unsigned i = 0; i < n; i++) s->pixels[i] = 0;
    }
    s->w = w;
    s->h = h;
    s->is_screen = 0;
}

/* Free pixel buffer. */
static inline void gfx_sprite_destroy(struct gfx_sprite *s) {
    if (s->pixels && !s->is_screen) {
        free(s->pixels);
    }
    s->pixels = 0;
}

/* Wrap the hardware framebuffer as a screen sprite. */
static inline struct gfx_sprite gfx_sprite_from_fb(struct gfx *g) {
    struct gfx_sprite s;
    s.pixels = g->real_fb ? g->real_fb : g->fb;
    s.w = g->width;
    s.h = g->height;
    s.is_screen = 1;
    s.pitch = g->pitch;
    s.rshift = g->rshift;
    s.gshift = g->gshift;
    s.bshift = g->bshift;
    s.bpp = g->bpp;
    return s;
}

/* ------------------------------------------------------------------ */
/*  Pixel helpers                                                      */
/* ------------------------------------------------------------------ */

/* Composite src RGBA over dst RGBA, store result in dst. */
static inline void composite_pixel(unsigned char *dst,
                                    uint8_t sr, uint8_t sg, uint8_t sb, uint8_t sa) {
    if (sa == 255) {
        dst[0] = sr; dst[1] = sg; dst[2] = sb; dst[3] = 255;
    } else if (sa == 0) {
        return;
    } else {
        int inv = 255 - sa;
        dst[0] = (sr * sa + dst[0] * inv) / 255;
        dst[1] = (sg * sa + dst[1] * inv) / 255;
        dst[2] = (sb * sa + dst[2] * inv) / 255;
        dst[3] = (255 * sa + dst[3] * inv) / 255;
    }
}

/* Pack RGBA into a native screen pixel (BPP 2, 3, or 4). */
static inline uint32_t pack_native(uint8_t r, uint8_t g, uint8_t b,
                                    uint8_t rshift, uint8_t gshift, uint8_t bshift,
                                    uint8_t bpp) {
    (void)bpp;
    return ((uint32_t)r << rshift) | ((uint32_t)g << gshift) | ((uint32_t)b << bshift);
}

/* Write a packed native pixel to the screen framebuffer. */
static inline void write_native(unsigned char *fb, uint32_t color, uint8_t bpp) {
    if (bpp == 4) {
        *(uint32_t *)fb = color;
    } else if (bpp == 3) {
        fb[0] = (uint8_t)(color & 0xFF);
        fb[1] = (uint8_t)((color >> 8) & 0xFF);
        fb[2] = (uint8_t)((color >> 16) & 0xFF);
    } else if (bpp == 2) {
        *(uint16_t *)fb = (uint16_t)color;
    }
}

/* ------------------------------------------------------------------ */
/*  Fill operations                                                     */
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

    if (dst->is_screen) {
        uint32_t color = pack_native(r, g, b, dst->rshift, dst->gshift, dst->bshift, dst->bpp);
        for (int row = 0; row < h; row++) {
            unsigned char *line = dst->pixels + (unsigned)(y + row) * dst->pitch + (unsigned)x * dst->bpp;
            for (int col = 0; col < w; col++) {
                write_native(line, color, dst->bpp);
                line += dst->bpp;
            }
        }
    } else {
        for (int row = 0; row < h; row++) {
            unsigned char *p = dst->pixels + ((unsigned)(y + row) * dst->w + (unsigned)x) * 4;
            for (int col = 0; col < w; col++) {
                composite_pixel(p, r, g, b, a);
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
/*  Blit (alpha-composited)                                            */
/* ------------------------------------------------------------------ */

static inline void gfx_sprite_blit(struct gfx_sprite *dst,
                                    const struct gfx_sprite *src,
                                    int dx, int dy) {
    if (!src->pixels || !dst->pixels) return;
    int sx = 0, sy = 0;
    int sw = (int)src->w, sh = (int)src->h;

    /* Clip source */
    if (dx < 0) { sx = -dx; sw += dx; dx = 0; }
    if (dy < 0) { sy = -dy; sh += dy; dy = 0; }
    if ((unsigned)dx >= dst->w || (unsigned)dy >= dst->h) return;
    if (dx + sw > (int)dst->w) sw = dst->w - dx;
    if (dy + sh > (int)dst->h) sh = dst->h - dy;
    if (sw <= 0 || sh <= 0) return;

    if (dst->is_screen) {
        for (int row = 0; row < sh; row++) {
            const unsigned char *src_p = src->pixels + ((unsigned)(sy + row) * src->w + (unsigned)sx) * 4;
            unsigned char *dst_p = dst->pixels + (unsigned)(dy + row) * dst->pitch + (unsigned)dx * dst->bpp;
            for (int col = 0; col < sw; col++) {
                uint8_t sa = src_p[3];
                if (sa > 128) {
                    uint32_t color = pack_native(src_p[0], src_p[1], src_p[2],
                                                  dst->rshift, dst->gshift, dst->bshift, dst->bpp);
                    write_native(dst_p, color, dst->bpp);
                }
                src_p += 4;
                dst_p += dst->bpp;
            }
        }
    } else {
        for (int row = 0; row < sh; row++) {
            const unsigned char *src_p = src->pixels + ((unsigned)(sy + row) * src->w + (unsigned)sx) * 4;
            unsigned char *dst_p = dst->pixels + ((unsigned)(dy + row) * dst->w + (unsigned)dx) * 4;
            for (int col = 0; col < sw; col++) {
                composite_pixel(dst_p, src_p[0], src_p[1], src_p[2], src_p[3]);
                src_p += 4;
                dst_p += 4;
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Text rendering                                                     */
/* ------------------------------------------------------------------ */

static inline void gfx_sprite_draw_str(struct gfx_sprite *dst, struct gfx *ctx,
                                        int x, int y, const char *str,
                                        uint8_t r, uint8_t g, uint8_t b,
                                        uint8_t a, int scale) {
    if (!str || !str[0]) return;

    if (dst->is_screen) {
        draw_str(ctx, x, y, str, r, g, b, scale);
        return;
    }

    /* Memory sprite: build a fake gfx pointing to the sprite buffer
       and use the font renderer. The font renderer writes a 32-bit
       packed value that sets R,G,B but leaves byte 3 (alpha) = 0.
       We fix up alpha in the text bounding box after rendering. */
    struct gfx fake;
    fake.fb = dst->pixels;
    fake.width = dst->w;
    fake.height = dst->h;
    fake.pitch = dst->w * 4;
    fake.bpp = 4;
    fake.rshift = 0;
    fake.gshift = 8;
    fake.bshift = 16;
    draw_str(&fake, x, y, str, r, g, b, scale);

    /* Re-assert alpha=255 in the text region (font renderer zeroed it). */
    int tw, th;
    if (g_font_ready && g_font) {
        tw = gfx_text_width(str, SCALE_PX(scale));
        th = SCALE_PX(scale) + 8;
    } else {
        int bm_scale = SCALE_PX(scale) > 12 ? 2 : 1;
        tw = (int)strlen(str) * 6 * bm_scale;
        th = 8 * bm_scale;
    }
    if (tw < 1) tw = 1;
    if (th < 1) th = 1;

    int x0 = x;
    int y0 = y;
    int x1 = x + tw;
    int y1 = y + th;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > (int)dst->w) x1 = (int)dst->w;
    if (y1 > (int)dst->h) y1 = (int)dst->h;

    for (int row = y0; row < y1; row++) {
        unsigned char *p = dst->pixels + ((unsigned)row * dst->w + (unsigned)x0) * 4 + 3;
        for (int col = x0; col < x1; col++) {
            *p = 255;
            p += 4;
        }
    }
}

#endif /* BOOTIE_SPRITE_H */
