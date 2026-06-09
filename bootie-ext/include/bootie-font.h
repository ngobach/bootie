#ifndef BOOTIE_FONT_H
#define BOOTIE_FONT_H

#include <bootie-gfx.h>
#include <schrift.h>

#if __has_include("font-ttf.h")
#  include "font-ttf.h"
#endif

/* ------------------------------------------------------------------ */
/*  Font state                                                        */
/* ------------------------------------------------------------------ */
static SFT_Font *g_font       = NULL;
static int       g_font_ready = 0;

static inline int gfx_font_load(void)
{
#if __has_include("font-ttf.h")
    if (g_font)
        sft_freefont(g_font);
    g_font = sft_loadmem(FONT_TTF_DATA, (size_t)FONT_TTF_SIZE);
    if (!g_font)
        return -1;
    g_font_ready = 1;
    return 0;
#else
    (void)sft_loadmem;
    g_font_ready = 0;
    return -1;
#endif
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
/*  Save a pointer to the bitmap draw_str before we override it.      */
/* ------------------------------------------------------------------ */
typedef void (*bitmap_draw_str_t)(struct gfx *, uint32_t, uint32_t,
                                   const char *, uint8_t, uint8_t, uint8_t,
                                   uint32_t);
static const bitmap_draw_str_t bitmap_draw_str = draw_str;

/* ------------------------------------------------------------------ */
/*  TTF glyph renderer                                                */
/* ------------------------------------------------------------------ */
static inline double gfx_font_render_glyph(struct gfx *ctx,
                                            int x, int y,
                                            SFT_UChar codepoint,
                                            uint8_t r, uint8_t g, uint8_t b,
                                            int px_size)
{
    if (!g_font)
        return 0.0;

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

    if (ctx->bpp == 4) {
        for (int row = 0; row < img.height; row++) {
            for (int col = 0; col < img.width; col++) {
                int alpha = buf[row * img.width + col];
                if (alpha == 0) continue;
                int px = origin_x + col;
                int py = origin_y + row;
                if ((unsigned int)px >= ctx->width || (unsigned int)py >= ctx->height)
                    continue;
                uint32_t *dst = (uint32_t *)(ctx->fb + (unsigned int)py * ctx->pitch) + px;
                if (alpha >= 254) {
                    *dst = ((uint32_t)r << ctx->rshift) |
                           ((uint32_t)g << ctx->gshift) |
                           ((uint32_t)b << ctx->bshift);
                } else {
                    uint32_t bg = *dst;
                    int inv = 255 - alpha;
                    uint8_t br = (uint8_t)((bg >> ctx->rshift) & 0xFF);
                    uint8_t bg_ = (uint8_t)((bg >> ctx->gshift) & 0xFF);
                    uint8_t bb = (uint8_t)((bg >> ctx->bshift) & 0xFF);
                    *dst = ((uint32_t)((alpha * r + inv * br) / 255) << ctx->rshift) |
                           ((uint32_t)((alpha * g + inv * bg_) / 255) << ctx->gshift) |
                           ((uint32_t)((alpha * b + inv * bb) / 255) << ctx->bshift);
                }
            }
        }
    } else {
        uint32_t color = 0;
        uint16_t c16 = 0;
        if (ctx->bpp == 3)
            color = ((uint32_t)r << ctx->rshift) |
                    ((uint32_t)g << ctx->gshift) |
                    ((uint32_t)b << ctx->bshift);
        else if (ctx->bpp == 2)
            c16 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
        else { free(buf); return mtx.advanceWidth; }

        for (int row = 0; row < img.height; row++) {
            for (int col = 0; col < img.width; col++) {
                if (buf[row * img.width + col] < 128) continue;
                int px = origin_x + col;
                int py = origin_y + row;
                if ((unsigned int)px >= ctx->width || (unsigned int)py >= ctx->height)
                    continue;
                uint8_t *dst = ctx->fb + (unsigned int)py * ctx->pitch + (unsigned int)px * ctx->bpp;
                if (ctx->bpp == 3) {
                    dst[0] = (uint8_t)(color & 0xFF);
                    dst[1] = (uint8_t)((color >> 8) & 0xFF);
                    dst[2] = (uint8_t)((color >> 16) & 0xFF);
                } else {
                    *(uint16_t *)dst = c16;
                }
            }
        }
    }

    free(buf);
    return mtx.advanceWidth;
}

/* ------------------------------------------------------------------ */
/*  TTF string renderer (UTF-8)                                       */
/*  Falls back to the bitmap draw_str when no TTF font is loaded.     */
/* ------------------------------------------------------------------ */
static inline void gfx_draw_str_ttf(struct gfx *ctx, int x, int y,
                                     const char *s,
                                     uint8_t r, uint8_t g, uint8_t b,
                                     int px_size)
{
    if (!g_font_ready || !g_font) {
        uint32_t scale = (uint32_t)(px_size > 12 ? 2 : 1);
        bitmap_draw_str(ctx, (uint32_t)x, (uint32_t)y, s, r, g, b, scale);
        return;
    }

    double em = (double)px_size;

    /* Compute baseline from a reference capital letter so the text
       top aligns with y regardless of font metrics. */
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

    /* ── Render ── */
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

        double adv = gfx_font_render_glyph(ctx, (int)pen_x, baseline_y, cp, r, g, b, (int)em);

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
/*  Falls back to bitmap estimate when no TTF font is loaded.          */
/* ------------------------------------------------------------------ */
static inline int gfx_text_width(const char *s, int px_size)
{
    if (!g_font_ready || !g_font) {
        int scale = px_size > 12 ? 2 : 1;
        return (int)(strlen(s) * 6 * (unsigned int)scale);
    }

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

/* Convert draw_str scale to pixel size (shared with gfx_text_width). */
#define SCALE_PX(s) ((s) <= 1 ? 16 : 28)

/* ------------------------------------------------------------------ */
/*  Override draw_str to use TTF font, mapping scale → px_size.       */
/* ------------------------------------------------------------------ */
#undef draw_str
#define draw_str(ctx, cx, cy, s, r, g, b, scale) \
    gfx_draw_str_ttf(ctx, (int)(cx), (int)(cy), s, r, g, b, SCALE_PX(scale))

/* ------------------------------------------------------------------ */
/*  draw_strf — sprintf + draw_str in one call                         */
/*  Usage:  draw_strf(ctx, x, y, r, g, b, scale, fmt, ...)            */
/* ------------------------------------------------------------------ */
#define draw_strf(ctx, cx, cy, r, g, b, scale, fmt, ...) do {         \
    char _buf[256];                                                    \
    sprintf(_buf, fmt, ##__VA_ARGS__);                                 \
    draw_str(ctx, cx, cy, _buf, r, g, b, scale);                      \
} while (0)

#endif /* BOOTIE_FONT_H */
