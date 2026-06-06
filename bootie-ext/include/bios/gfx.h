#ifndef BIOS_GFX_H
#define BIOS_GFX_H

#include <bootprog.h>
#include <stdint.h>

/* ------------------------------------------------------------------ */
/*  VBE structures                                                      */
/* ------------------------------------------------------------------ */
struct vbe_mode_info {
    uint16_t ModeAttributes;
    uint8_t  WinAAttributes;
    uint8_t  WinBAttributes;
    uint16_t WinGranularity;
    uint16_t WinSize;
    uint16_t WinASegment;
    uint16_t WinBSegment;
    uint32_t WinFuncPtr;
    uint16_t BytesPerScanline;

    uint16_t XResolution;
    uint16_t YResolution;
    uint8_t  XCharSize;
    uint8_t  YCharSize;
    uint8_t  NumberOfPlanes;
    uint8_t  BitsPerPixel;
    uint8_t  NumberOfBanks;
    uint8_t  MemoryModel;
    uint8_t  BankSize;
    uint8_t  NumberOfImagePages;
    uint8_t  Reserved1;

    uint8_t  RedMaskSize;
    uint8_t  RedFieldPosition;
    uint8_t  GreenMaskSize;
    uint8_t  GreenFieldPosition;
    uint8_t  BlueMaskSize;
    uint8_t  BlueFieldPosition;
    uint8_t  RsvdMaskSize;
    uint8_t  RsvdFieldPosition;
    uint8_t  DirectColorModeInfo;

    uint32_t PhysBasePtr;
    uint32_t Reserved2;
    uint16_t Reserved3;

    uint16_t LinBytesPerScanline;
    uint8_t  BnkNumberOfImagePages;
    uint8_t  LinNumberOfImagePages;
    uint8_t  LinRedMaskSize;
    uint8_t  LinRedFieldPosition;
    uint8_t  LinGreenMaskSize;
    uint8_t  LinGreenFieldPosition;
    uint8_t  LinBlueMaskSize;
    uint8_t  LinBlueFieldPosition;
    uint8_t  LinRsvdMaskSize;
    uint8_t  LinRsvdFieldPosition;
    uint32_t MaxPixelClock;
    uint8_t  Reserved4[189];
} __attribute__((packed));

struct vbe_driver_info {
    uint8_t  VBESignature[4];
    uint16_t VBEVersion;
    uint32_t OEMStringPtr;
    uint32_t Capabilities;
    uint32_t VideoModePtr;
    uint16_t TotalMemory;
    uint16_t OemSoftwareRev;
    uint32_t OemVendorNamePtr;
    uint32_t OemProductNamePtr;
    uint32_t OemProductRevPtr;
    uint8_t  Reserved[222];
    uint8_t  OemDATA[256];
} __attribute__((packed));

struct realmode_regs {
    unsigned long edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned long gs, fs, es, ds, ss, eip, cs, eflags;
};

/* ------------------------------------------------------------------ */
/*  Graphics Context Structure                                        */
/* ------------------------------------------------------------------ */
struct gfx {
    uint8_t  *fb;
    uint32_t  width;
    uint32_t  height;
    uint32_t  pitch;
    uint32_t  bpp;
    uint8_t   rshift;
    uint8_t   gshift;
    uint8_t   bshift;
};

static inline uint32_t gfx_width(const struct gfx *g) {
    return g->width;
}

static inline uint32_t gfx_height(const struct gfx *g) {
    return g->height;
}

/* ------------------------------------------------------------------ */
/*  5x7 bitmap font (printable ASCII 0x20..0x7E)                       */
/* ------------------------------------------------------------------ */
static const uint8_t font5x7[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, /* ' ' */
    {0x00,0x00,0x5F,0x00,0x00}, /* '!' */
    {0x00,0x07,0x00,0x07,0x00}, /* '"' */
    {0x14,0x7F,0x14,0x7F,0x14}, /* '#' */
    {0x24,0x2A,0x7F,0x2A,0x12}, /* '$' */
    {0x23,0x13,0x08,0x64,0x62}, /* '%' */
    {0x36,0x49,0x55,0x22,0x50}, /* '&' */
    {0x00,0x05,0x03,0x00,0x00}, /* '\'' */
    {0x00,0x1C,0x22,0x41,0x00}, /* '(' */
    {0x00,0x41,0x22,0x1C,0x00}, /* ')' */
    {0x14,0x08,0x3E,0x08,0x14}, /* '*' */
    {0x08,0x08,0x3E,0x08,0x08}, /* '+' */
    {0x00,0x50,0x30,0x00,0x00}, /* ',' */
    {0x08,0x08,0x08,0x08,0x08}, /* '-' */
    {0x00,0x60,0x60,0x00,0x00}, /* '.' */
    {0x20,0x10,0x08,0x04,0x02}, /* '/' */
    {0x3E,0x51,0x49,0x45,0x3E}, /* '0' */
    {0x00,0x42,0x7F,0x40,0x00}, /* '1' */
    {0x42,0x61,0x51,0x49,0x46}, /* '2' */
    {0x21,0x41,0x45,0x4B,0x31}, /* '3' */
    {0x18,0x14,0x12,0x7F,0x10}, /* '4' */
    {0x27,0x45,0x45,0x45,0x39}, /* '5' */
    {0x3C,0x4A,0x49,0x49,0x30}, /* '6' */
    {0x01,0x71,0x09,0x05,0x03}, /* '7' */
    {0x36,0x49,0x49,0x49,0x36}, /* '8' */
    {0x06,0x49,0x49,0x29,0x1E}, /* '9' */
    {0x00,0x36,0x36,0x00,0x00}, /* ':' */
    {0x00,0x56,0x36,0x00,0x00}, /* ';' */
    {0x08,0x14,0x22,0x41,0x00}, /* '<' */
    {0x14,0x14,0x14,0x14,0x14}, /* '=' */
    {0x00,0x41,0x22,0x14,0x08}, /* '>' */
    {0x02,0x01,0x51,0x09,0x06}, /* '?' */
    {0x32,0x49,0x79,0x41,0x3E}, /* '@' */
    {0x7E,0x11,0x11,0x11,0x7E}, /* 'A' */
    {0x7F,0x49,0x49,0x49,0x36}, /* 'B' */
    {0x3E,0x41,0x41,0x41,0x22}, /* 'C' */
    {0x7F,0x41,0x41,0x22,0x1C}, /* 'D' */
    {0x7F,0x49,0x49,0x49,0x41}, /* 'E' */
    {0x7F,0x09,0x09,0x09,0x01}, /* 'F' */
    {0x3E,0x41,0x49,0x49,0x7A}, /* 'G' */
    {0x7F,0x08,0x08,0x08,0x7F}, /* 'H' */
    {0x00,0x41,0x7F,0x41,0x00}, /* 'I' */
    {0x20,0x40,0x41,0x3F,0x01}, /* 'J' */
    {0x7F,0x08,0x14,0x22,0x41}, /* 'K' */
    {0x7F,0x40,0x40,0x40,0x40}, /* 'L' */
    {0x7F,0x02,0x0C,0x02,0x7F}, /* 'M' */
    {0x7F,0x04,0x08,0x10,0x7F}, /* 'N' */
    {0x3E,0x41,0x41,0x41,0x3E}, /* 'O' */
    {0x7F,0x09,0x09,0x09,0x06}, /* 'P' */
    {0x3E,0x41,0x51,0x21,0x5E}, /* 'Q' */
    {0x7F,0x09,0x19,0x29,0x46}, /* 'R' */
    {0x46,0x49,0x49,0x49,0x31}, /* 'S' */
    {0x01,0x01,0x7F,0x01,0x01}, /* 'T' */
    {0x3F,0x40,0x40,0x40,0x3F}, /* 'U' */
    {0x1F,0x20,0x40,0x20,0x1F}, /* 'V' */
    {0x3F,0x40,0x38,0x40,0x3F}, /* 'W' */
    {0x63,0x14,0x08,0x14,0x63}, /* 'X' */
    {0x07,0x08,0x70,0x08,0x07}, /* 'Y' */
    {0x61,0x51,0x49,0x45,0x43}, /* 'Z' */
    {0x00,0x7F,0x41,0x41,0x00}, /* '[' */
    {0x02,0x04,0x08,0x10,0x20}, /* '\\' */
    {0x00,0x41,0x41,0x7F,0x00}, /* ']' */
    {0x04,0x02,0x01,0x02,0x04}, /* '^' */
    {0x40,0x40,0x40,0x40,0x40}, /* '_' */
    {0x00,0x01,0x02,0x04,0x00}, /* '`' */
    {0x20,0x54,0x54,0x54,0x78}, /* 'a' */
    {0x7F,0x48,0x44,0x44,0x38}, /* 'b' */
    {0x38,0x44,0x44,0x44,0x20}, /* 'c' */
    {0x38,0x44,0x44,0x48,0x7F}, /* 'd' */
    {0x38,0x54,0x54,0x54,0x18}, /* 'e' */
    {0x08,0x7E,0x09,0x01,0x02}, /* 'f' */
    {0x0C,0x52,0x52,0x52,0x3E}, /* 'g' */
    {0x7F,0x08,0x04,0x04,0x78}, /* 'h' */
    {0x00,0x44,0x7D,0x40,0x00}, /* 'i' */
    {0x20,0x40,0x44,0x3D,0x00}, /* 'j' */
    {0x7F,0x10,0x28,0x44,0x00}, /* 'k' */
    {0x00,0x41,0x7F,0x40,0x00}, /* 'l' */
    {0x7C,0x04,0x18,0x04,0x78}, /* 'm' */
    {0x7C,0x08,0x04,0x04,0x78}, /* 'n' */
    {0x38,0x44,0x44,0x44,0x38}, /* 'o' */
    {0x7C,0x14,0x14,0x14,0x08}, /* 'p' */
    {0x08,0x14,0x14,0x18,0x7C}, /* 'q' */
    {0x7C,0x08,0x04,0x04,0x08}, /* 'r' */
    {0x48,0x54,0x54,0x54,0x20}, /* 's' */
    {0x04,0x3F,0x44,0x40,0x20}, /* 't' */
    {0x3C,0x40,0x40,0x40,0x7C}, /* 'u' */
    {0x1C,0x20,0x40,0x20,0x1C}, /* 'v' */
    {0x3C,0x40,0x30,0x40,0x3C}, /* 'w' */
    {0x44,0x28,0x10,0x28,0x44}, /* 'x' */
    {0x0C,0x50,0x50,0x50,0x3C}, /* 'y' */
    {0x44,0x64,0x54,0x4C,0x44}, /* 'z' */
    {0x00,0x08,0x36,0x41,0x00}, /* '{' */
    {0x00,0x00,0x7F,0x00,0x00}, /* '|' */
    {0x00,0x41,0x36,0x08,0x00}, /* '}' */
    {0x10,0x08,0x08,0x10,0x08}, /* '~' */
};

/* ------------------------------------------------------------------ */
/*  Low-level pixel & BIOS helpers                                      */
/* ------------------------------------------------------------------ */
static int bios_int10(unsigned long eax, unsigned long ebx,
                      unsigned long ecx, unsigned long edx,
                      unsigned long es,  unsigned long edi) {
    struct realmode_regs r = {
        edi, 0, 0, (unsigned long)-1,
        ebx, edx, ecx, eax,
        (unsigned long)-1, (unsigned long)-1,
        es, (unsigned long)-1, (unsigned long)-1,
        0xFFFF10CD, (unsigned long)-1, (unsigned long)-1
    };
    realmode_run((long)&r);
    return (int)r.eax;
}

static int bios_checkkey(void) {
    struct realmode_regs r = {
        0, 0, 0, (unsigned long)-1, 0, 0, 0, 0x0100,
        (unsigned long)-1, (unsigned long)-1,
        (unsigned long)-1, (unsigned long)-1,
        (unsigned long)-1, 0xFFFF16CD,
        (unsigned long)-1, (unsigned long)-1
    };
    realmode_run((long)&r);
    return (r.eflags & (1 << 6)) == 0;
}

static int bios_getkey(void) {
    struct realmode_regs r = {
        0, 0, 0, (unsigned long)-1, 0, 0, 0, 0x0000,
        (unsigned long)-1, (unsigned long)-1,
        (unsigned long)-1, (unsigned long)-1,
        (unsigned long)-1, 0xFFFF16CD,
        (unsigned long)-1, (unsigned long)-1
    };
    realmode_run((long)&r);
    uint8_t ascii = r.eax & 0xFF;
    return ascii ? ascii : (int)((r.eax >> 8) & 0xFF) << 8;
}

static inline uint32_t rm_to_flat(uint32_t seg_ofs) {
    return ((seg_ofs >> 16) << 4) + (uint16_t)seg_ofs;
}

static int get_driver_info(struct vbe_driver_info *drv) {
    struct vbe_driver_info *di = (struct vbe_driver_info *)0x20000;
    memset(di, 0, sizeof(*di));
    di->VBESignature[0] = 'V'; di->VBESignature[1] = 'B';
    di->VBESignature[2] = 'E'; di->VBESignature[3] = '2';
    if ((bios_int10(0x4F00, 0, 0, 0, 0x2000, 0) & 0xFF) != 0x4F)
        return 0;
    if (memcmp(di->VBESignature, "VESA", 4) != 0) return 0;
    if (di->VBEVersion < 0x200) return 0;
    di->VideoModePtr = rm_to_flat(di->VideoModePtr);
    memmove(drv, di, sizeof(*di));
    return 1;
}

static int get_mode_info(uint16_t mode, struct vbe_mode_info *mi_out) {
    if (!mode || mode == 0xFFFF) return 0;
    struct vbe_mode_info *mi = (struct vbe_mode_info *)0x20400;
    memset(mi, 0, sizeof(*mi));
    if ((bios_int10(0x4F01, 0, mode, 0, 0x2000, 1024) & 0xFF) != 0x4F) return 0;
    if (!(mi->ModeAttributes & 1))    return 0;  /* not supported */
    if (!(mi->ModeAttributes & 0x80)) return 0;  /* no LFB        */
    if (mi->PhysBasePtr == 0)         return 0;
    memmove(mi_out, mi, sizeof(*mi));
    return 1;
}

/* ------------------------------------------------------------------ */
/*  Framebuffer pixel writer                                            */
/* ------------------------------------------------------------------ */
static inline __attribute__((always_inline)) void put_pixel(struct gfx *ctx, uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b) {
    if (x >= ctx->width || y >= ctx->height) return;
    uint8_t *p = ctx->fb + y * ctx->pitch + x * ctx->bpp;
    uint32_t color = ((uint32_t)r << ctx->rshift) |
                     ((uint32_t)g << ctx->gshift) |
                     ((uint32_t)b << ctx->bshift);
    if (ctx->bpp == 4) {
        *((uint32_t *)p) = color;
    } else if (ctx->bpp == 3) {
        p[0] = color & 0xFF;
        p[1] = (color >> 8) & 0xFF;
        p[2] = (color >> 16) & 0xFF;
    } else if (ctx->bpp == 2) {
        /* 5-6-5 approximation */
        uint16_t c16 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
        *((uint16_t *)p) = c16;
    }
}

/* Fill a rectangle with a solid colour using fast row writes */
static void fill_rect(struct gfx *ctx, uint32_t x, uint32_t y,
                      uint32_t w, uint32_t h,
                      uint8_t r, uint8_t g, uint8_t b) {
    if (x >= ctx->width || y >= ctx->height) return;
    uint32_t x_end = (x + w > ctx->width) ? ctx->width : (x + w);
    uint32_t y_end = (y + h > ctx->height) ? ctx->height : (y + h);
    if (x_end <= x || y_end <= y) return;

    uint32_t color = ((uint32_t)r << ctx->rshift) |
                     ((uint32_t)g << ctx->gshift) |
                     ((uint32_t)b << ctx->bshift);

    if (ctx->bpp == 4) {
        for (uint32_t row = y; row < y_end; row++) {
            uint32_t *p = (uint32_t *)(ctx->fb + row * ctx->pitch) + x;
            uint32_t count = x_end - x;
            while (count >= 8) {
                p[0] = color; p[1] = color; p[2] = color; p[3] = color;
                p[4] = color; p[5] = color; p[6] = color; p[7] = color;
                p += 8;
                count -= 8;
            }
            while (count > 0) {
                *p++ = color;
                count--;
            }
        }
    } else if (ctx->bpp == 3) {
        uint8_t b0 = color & 0xFF;
        uint8_t b1 = (color >> 8) & 0xFF;
        uint8_t b2 = (color >> 16) & 0xFF;
        for (uint32_t row = y; row < y_end; row++) {
            uint8_t *p = ctx->fb + row * ctx->pitch + x * 3;
            uint32_t count = x_end - x;
            for (uint32_t col = 0; col < count; col++) {
                p[0] = b0;
                p[1] = b1;
                p[2] = b2;
                p += 3;
            }
        }
    } else if (ctx->bpp == 2) {
        uint16_t c16 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
        for (uint32_t row = y; row < y_end; row++) {
            uint16_t *p = (uint16_t *)(ctx->fb + row * ctx->pitch) + x;
            uint32_t count = x_end - x;
            while (count >= 8) {
                p[0] = c16; p[1] = c16; p[2] = c16; p[3] = c16;
                p[4] = c16; p[5] = c16; p[6] = c16; p[7] = c16;
                p += 8;
                count -= 8;
            }
            while (count > 0) {
                *p++ = c16;
                count--;
            }
        }
    }
}

/* Draw a single character (scale: each dot = scale x scale pixels) */
static void draw_char(struct gfx *ctx, uint32_t cx, uint32_t cy, char ch,
                      uint8_t r, uint8_t g, uint8_t b, uint32_t scale) {
    if (ch < 0x20 || ch > 0x7E) ch = '?';
    const uint8_t *glyph = font5x7[ch - 0x20];
    for (uint32_t col = 0; col < 5; col++) {
        for (uint32_t row = 0; row < 7; row++) {
            if (glyph[col] & (1 << row)) {
                for (uint32_t sy = 0; sy < scale; sy++)
                    for (uint32_t sx = 0; sx < scale; sx++)
                        put_pixel(ctx, cx + col*scale + sx,
                                  cy + row*scale + sy,
                                  r, g, b);
            }
        }
    }
}

/* Draw a string */
static void draw_str(struct gfx *ctx, uint32_t cx, uint32_t cy, const char *s,
                     uint8_t r, uint8_t g, uint8_t b, uint32_t scale) {
    while (*s) {
        draw_char(ctx, cx, cy, *s, r, g, b, scale);
        cx += (5 + 1) * scale; /* 5 columns + 1 gap */
        s++;
    }
}

/* ------------------------------------------------------------------ */
/*  Delay (BIOS timer tick ≈ 55 ms)                                    */
/* ------------------------------------------------------------------ */
static void delay_ms(unsigned int ms) {
    unsigned int ticks = (ms + 54) / 55;
    unsigned long start = *(volatile unsigned long *)0x46c;
    while ((*(volatile unsigned long *)0x46c - start) < ticks) {}
}

/* ------------------------------------------------------------------ */
/*  Platform Agnostic Wrappers                                        */
/* ------------------------------------------------------------------ */
static inline int gfx_init(struct gfx *ctx) {
    /* --- find best VBE mode --- */
    struct vbe_driver_info drv;
    if (!get_driver_info(&drv)) {
        printf("VBE 2.0 not supported\n");
        return 0;
    }

    uint16_t *modes = (uint16_t *)drv.VideoModePtr;
    uint16_t best_mode  = 0;
    int      best_score = -1;
    struct vbe_mode_info best_mi;

    for (int idx = 0; modes[idx] != 0xFFFF; idx++) {
        uint16_t mode = modes[idx];
        struct vbe_mode_info mi;
        if (!get_mode_info(mode, &mi)) continue;

        /* skip non-packed-pixel memory models */
        if (mi.MemoryModel != 4 && mi.MemoryModel != 6) continue;

        int score = 0;
        if      (mi.BitsPerPixel == 32) score += 100;
        else if (mi.BitsPerPixel == 24) score +=  90;
        else if (mi.BitsPerPixel == 16) score +=  50;
        else continue;

        if      (mi.XResolution == 1024 && mi.YResolution == 768)  score += 60;
        else if (mi.XResolution ==  800 && mi.YResolution == 600)  score += 50;
        else if (mi.XResolution ==  640 && mi.YResolution == 480)  score += 30;
        else score += 5;

        if (score > best_score) {
            best_score = score;
            best_mode  = mode;
            best_mi    = mi;
        }
    }

    if (!best_mode) {
        printf("No suitable VBE mode found\n");
        return 0;
    }

    printf("Setting VBE mode 0x%X (%dx%d, %d bpp)\n",
           (int)best_mode,
           (int)best_mi.XResolution, (int)best_mi.YResolution,
           (int)best_mi.BitsPerPixel);

    if ((bios_int10(0x4F02, 0x4000 | best_mode, 0, 0, (unsigned long)-1, 0) & 0xFF) != 0x4F) {
        printf("Failed to set VBE mode\n");
        return 0;
    }

    /* --- set up framebuffer fields in ctx --- */
    ctx->fb        = (uint8_t *)best_mi.PhysBasePtr;
    ctx->width  = best_mi.XResolution;
    ctx->height = best_mi.YResolution;
    ctx->pitch  = best_mi.LinBytesPerScanline ? best_mi.LinBytesPerScanline
                                            : best_mi.BytesPerScanline;
    ctx->bpp    = (best_mi.BitsPerPixel + 7) / 8;

    /* determine channel shifts from VBE colour mask info */
    ctx->rshift = best_mi.LinRedFieldPosition   ? best_mi.LinRedFieldPosition
                                              : best_mi.RedFieldPosition;
    ctx->gshift = best_mi.LinGreenFieldPosition ? best_mi.LinGreenFieldPosition
                                              : best_mi.GreenFieldPosition;
    ctx->bshift = best_mi.LinBlueFieldPosition  ? best_mi.LinBlueFieldPosition
                                              : best_mi.BlueFieldPosition;

    printf("fb=0x%08X pitch=%u bpp=%u rgb_shifts=(%u,%u,%u)\n",
           (unsigned int)ctx->fb, (unsigned int)ctx->pitch, (unsigned int)ctx->bpp,
           (unsigned int)ctx->rshift, (unsigned int)ctx->gshift, (unsigned int)ctx->bshift);

    return 1;
}

static inline void gfx_close(struct gfx *ctx) {
    (void)ctx;
    /* --- restore text mode --- */
    if (graphics_inited) {
        if (current_term->STARTUP)
            ((int (*)(int))current_term->STARTUP)(0);
    } else {
        bios_int10(3, 0, 0, 0, (unsigned long)-1, 0);
    }
    cls();
}

static inline int gfx_checkkey(struct gfx *ctx) {
    (void)ctx;
    return bios_checkkey();
}

static inline int gfx_getkey(struct gfx *ctx) {
    (void)ctx;
    return bios_getkey();
}

static inline void gfx_delay_ms(struct gfx *ctx, unsigned int ms) {
    (void)ctx;
    delay_ms(ms);
}

#endif /* BIOS_GFX_H */
