#ifndef BOOTIE_FONT_H
#define BOOTIE_FONT_H

#include <bootie-gfx.h>
#include <schrift.h>
#include "bootie-fonts.h"

/* ------------------------------------------------------------------ */
/*  Font state                                                        */
/* ------------------------------------------------------------------ */
static SFT_Font *g_font;
static int       g_font_ready;

static inline int gfx_font_load(void)
{
    if (g_font)
        sft_freefont(g_font);
    g_font = sft_loadmem(FONT_TTF_DATA, (size_t)FONT_TTF_SIZE);
    if (!g_font)
        return -1;
    g_font_ready = 1;
    return 0;
}

static inline void gfx_font_unload(void)
{
    if (g_font) {
        sft_freefont(g_font);
        g_font = NULL;
    }
    g_font_ready = 0;
}

/* ------------------------------------------------------------------ */
/*  Include the libschrift rasteriser in the same TU as the module.    */
/*  This is done here (rather than compiled separately) so that it     */
/*  inherits the GRUB4DOS macros (malloc, free, memset, memmove,       */
/*  memcmp) which are not available as linkable symbols.               */
/* ------------------------------------------------------------------ */
#include "../libs/schrift/schrift.c"

/* ------------------------------------------------------------------ */
/*  TTF glyph renderer                                                */
/* ------------------------------------------------------------------ */
static inline double gfx_font_render_glyph(struct gfx_sprite *spr,
                                            int x, int y,
                                            SFT_UChar codepoint,
                                            uint8_t r, uint8_t g, uint8_t b,
                                            int px_size)
{
    SFT sft;
    sft.font    = g_font;
    sft.xScale  = (double)px_size;
    sft.yScale  = (double)px_size;
    sft.xOffset = 0.0;
    sft.yOffset = 0.0;
    sft.flags   = SFT_DOWNWARD_Y;

    SFT_Glyph glyph;
    if (sft_lookup(&sft, codepoint, &glyph) || !glyph)
        return 0.0;

    SFT_GMetrics mtx;
    if (sft_gmetrics(&sft, glyph, &mtx))
        return 0.0;

    if (mtx.minWidth <= 0 || mtx.minHeight <= 0)
        return mtx.advanceWidth;

    SFT_Image img;
    img.width  = mtx.minWidth;
    img.height = mtx.minHeight;
    uint8_t *buf = (uint8_t *)malloc((unsigned int)(img.width * img.height));
    if (!buf)
        return mtx.advanceWidth;
    img.pixels = buf;

    if (sft_render(&sft, glyph, img)) {
        free(buf);
        return mtx.advanceWidth;
    }

    int origin_x = x + (int)mtx.leftSideBearing;
    int origin_y = y + mtx.yOffset;

    /* Sprites are always RGBA (rshift=0, gshift=8, bshift=16, bpp=4) */
    for (int row = 0; row < img.height; row++) {
        for (int col = 0; col < img.width; col++) {
            int alpha = buf[row * img.width + col];
            if (alpha == 0) continue;
            int px = origin_x + col;
            int py = origin_y + row;
            if ((unsigned int)px >= spr->w || (unsigned int)py >= spr->h)
                continue;
            uint32_t *dst = (uint32_t *)(spr->pixels + (unsigned int)py * spr->w * 4) + px;
            if (alpha >= 254) {
                *dst = ((uint32_t)r) |
                       ((uint32_t)g << 8) |
                       ((uint32_t)b << 16) |
                       ((uint32_t)255 << 24);
            } else {
                uint32_t existing = *dst;
                uint8_t er = (uint8_t)(existing & 0xFF);
                uint8_t eg = (uint8_t)((existing >> 8) & 0xFF);
                uint8_t eb = (uint8_t)((existing >> 16) & 0xFF);
                uint8_t ea = (uint8_t)((existing >> 24) & 0xFF);
                int inv = 255 - alpha;
                uint8_t fr = (uint8_t)((r * alpha + er * inv + 128) >> 8);
                uint8_t fg = (uint8_t)((g * alpha + eg * inv + 128) >> 8);
                uint8_t fb_ = (uint8_t)((b * alpha + eb * inv + 128) >> 8);
                uint8_t fa = (uint8_t)((alpha * 255 + ea * inv + 128) >> 8);
                *dst = ((uint32_t)fr) |
                       ((uint32_t)fg << 8) |
                       ((uint32_t)fb_ << 16) |
                       ((uint32_t)fa << 24);
            }
        }
    }

    free(buf);
    return mtx.advanceWidth;
}

/* ------------------------------------------------------------------ */
/*  TTF string renderer (UTF-8)                                       */
/* ------------------------------------------------------------------ */
static inline void gfx_draw_str_ttf(struct gfx_sprite *spr, int x, int y,
                                     const char *s,
                                     uint8_t r, uint8_t g, uint8_t b,
                                     int px_size)
{
    double em = (double)px_size;

    int baseline_y = y + (int)(em * 0.8);
    {
        SFT msft;
        msft.font    = g_font;
        msft.xScale  = em;
        msft.yScale  = em;
        msft.xOffset = 0.0;
        msft.yOffset = 0.0;
        msft.flags   = SFT_DOWNWARD_Y;
        SFT_Glyph ref;
        if (sft_lookup(&msft, (SFT_UChar)0x48, &ref) == 0 && ref) {
            SFT_GMetrics refm;
            if (sft_gmetrics(&msft, ref, &refm) == 0)
                baseline_y = y - refm.yOffset;
        }
    }

    double pen_x = (double)x;
    SFT_UChar prev = 0;

    while (*s) {
        SFT_UChar cp;
        unsigned char c = (unsigned char)*s;
        if ((c & 0x80) == 0)            { cp = c; s++; }
        else if ((c & 0xE0) == 0xC0 && (s[1] & 0xC0) == 0x80) {
            cp = ((uint8_t)s[0] & 0x1F) << 6 | ((uint8_t)s[1] & 0x3F);
            s += 2;
        } else if ((c & 0xF0) == 0xE0 && (s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80) {
            cp = ((uint8_t)s[0] & 0x0F) << 12 | ((uint8_t)s[1] & 0x3F) << 6 | ((uint8_t)s[2] & 0x3F);
            s += 3;
        } else if ((c & 0xF8) == 0xF0 && (s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80 && (s[3] & 0xC0) == 0x80) {
            cp = ((uint8_t)s[0] & 0x07) << 18 | ((uint8_t)s[1] & 0x3F) << 12 | ((uint8_t)s[2] & 0x3F) << 6 | ((uint8_t)s[3] & 0x3F);
            s += 4;
        } else { s++; continue; }

        if (cp == '\n') { pen_x = (double)x; prev = 0; continue; }

        double adv = gfx_font_render_glyph(spr, (int)pen_x, baseline_y, cp, r, g, b, (int)em);

        if (prev) {
            SFT_Glyph prev_g, cur_g;
            SFT kersft;
            kersft.font    = g_font;
            kersft.xScale  = em;
            kersft.yScale  = em;
            kersft.xOffset = 0.0;
            kersft.yOffset = 0.0;
            kersft.flags   = SFT_DOWNWARD_Y;
            if (sft_lookup(&kersft, prev, &prev_g) == 0 && prev_g &&
                sft_lookup(&kersft, cp, &cur_g) == 0 && cur_g) {
                SFT_Kerning kern;
                if (sft_kerning(&kersft, prev_g, cur_g, &kern) == 0)
                    adv += kern.xShift;
            }
        }

        pen_x += adv;
        prev = cp;
    }
}

/* ------------------------------------------------------------------ */
/*  Proportional text width measurement                                */
/*  Returns the pixel width of a UTF-8 string at the given px_size.    */
/* ------------------------------------------------------------------ */
static inline int gfx_text_width(const char *s, int px_size)
{
    double em = (double)px_size;
    SFT msft;
    msft.font    = g_font;
    msft.xScale  = em;
    msft.yScale  = em;
    msft.xOffset = 0.0;
    msft.yOffset = 0.0;
    msft.flags   = SFT_DOWNWARD_Y;

    double w = 0.0;
    SFT_UChar prev = 0;
    SFT_Glyph prev_gid = 0;

    while (*s) {
        SFT_UChar cp;
        unsigned char c = (unsigned char)*s;
        if ((c & 0x80) == 0)            { cp = c; s++; }
        else if ((c & 0xE0) == 0xC0 && (s[1] & 0xC0) == 0x80) {
            cp = ((uint8_t)s[0] & 0x1F) << 6 | ((uint8_t)s[1] & 0x3F); s += 2;
        } else if ((c & 0xF0) == 0xE0 && (s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80) {
            cp = ((uint8_t)s[0] & 0x0F) << 12 | ((uint8_t)s[1] & 0x3F) << 6 | ((uint8_t)s[2] & 0x3F); s += 3;
        } else if ((c & 0xF8) == 0xF0 && (s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80 && (s[3] & 0xC0) == 0x80) {
            cp = ((uint8_t)s[0] & 0x07) << 18 | ((uint8_t)s[1] & 0x3F) << 12 | ((uint8_t)s[2] & 0x3F) << 6 | ((uint8_t)s[3] & 0x3F); s += 4;
        } else { s++; continue; }
        if (cp == '\n') { w = 0.0; prev = 0; prev_gid = 0; continue; }

        SFT_Glyph gid;
        if (sft_lookup(&msft, cp, &gid) || !gid) continue;
        SFT_GMetrics m;
        if (sft_gmetrics(&msft, gid, &m)) continue;
        double adv = m.advanceWidth;
        if (prev && prev_gid) {
            SFT_Kerning kern;
            if (sft_kerning(&msft, prev_gid, gid, &kern) == 0)
                adv += kern.xShift;
        }
        w += adv;
        prev = cp;
        prev_gid = gid;
    }
    return (int)(w + 0.5);
}

/* ------------------------------------------------------------------ */
/*  Override draw_str to use TTF font with px_size instead of scale.   */
/* ------------------------------------------------------------------ */
#undef draw_str
#define draw_str(spr, cx, cy, s, r, g, b, px_size) \
    gfx_draw_str_ttf((struct gfx_sprite *)(spr), (int)(cx), (int)(cy), s, r, g, b, px_size)

/* ------------------------------------------------------------------ */
/*  draw_strf — sprintf + draw_str in one call                         */
/*  Usage:  draw_strf(ctx, x, y, r, g, b, px_size, fmt, ...)          */
/* ------------------------------------------------------------------ */
#define draw_strf(ctx, cx, cy, r, g, b, px_size, fmt, ...) do {       \
    char _buf[256];                                                    \
    sprintf(_buf, fmt, ##__VA_ARGS__);                                 \
    draw_str(ctx, cx, cy, _buf, r, g, b, px_size);                    \
} while (0)

/* Centered variants — compute the pixel width of the *actual* rendered
   text and subtract half from cx, so cx becomes the center point. */
#define draw_strf_centered(ctx, cx, cy, r, g, b, px_size, fmt, ...) do { \
    char _buf[256];                                                    \
    int _w;                                                            \
    sprintf(_buf, fmt, ##__VA_ARGS__);                                 \
    _w = gfx_text_width(_buf, px_size);                                \
    draw_str(ctx, (int)(cx) - _w / 2, cy, _buf, r, g, b, px_size);    \
} while (0)


#endif /* BOOTIE_FONT_H */
