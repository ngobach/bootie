#ifndef BIOS_GFX_H
#define BIOS_GFX_H

#include <bootie.h>
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
    uint8_t  *real_fb;
    void     *backbuf;
};

static inline uint32_t gfx_width(const struct gfx *g) {
    return g->width;
}

static inline uint32_t gfx_height(const struct gfx *g) {
    return g->height;
}


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
            __asm__ __volatile__("rep stosl" : "+D"(p), "+c"(count) : "a"(color) : "memory");
        }
    } else if (ctx->bpp == 3) {
        uint8_t b0 = color & 0xFF;
        uint8_t b1 = (color >> 8) & 0xFF;
        uint8_t b2 = (color >> 16) & 0xFF;
        for (uint32_t row = y; row < y_end; row++) {
            uint8_t *p = ctx->fb + row * ctx->pitch + x * 3;
            uint32_t count = x_end - x;
            while (count >= 4) {
                p[0] = b0; p[1] = b1; p[2] = b2;
                p[3] = b0; p[4] = b1; p[5] = b2;
                p[6] = b0; p[7] = b1; p[8] = b2;
                p[9] = b0; p[10] = b1; p[11] = b2;
                p += 12; count -= 4;
            }
            while (count > 0) {
                p[0] = b0; p[1] = b1; p[2] = b2;
                p += 3; count--;
            }
        }
    } else if (ctx->bpp == 2) {
        uint16_t c16 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
        for (uint32_t row = y; row < y_end; row++) {
            uint16_t *p = (uint16_t *)(ctx->fb + row * ctx->pitch) + x;
            uint32_t count = x_end - x;
            __asm__ __volatile__("rep stosw" : "+D"(p), "+c"(count) : "a"(c16) : "memory");
        }
    }
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

        if (mi.XResolution <= 800) {
            score += 1000;
            if      (mi.XResolution ==  800 && mi.YResolution == 600)  score += 50;
            else if (mi.XResolution ==  640 && mi.YResolution == 480)  score += 30;
            else score += 5;
        } else {
            if      (mi.XResolution == 1024 && mi.YResolution == 768)  score += 60;
            else score += 5;
        }

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
    ctx->real_fb = NULL;
    ctx->backbuf = NULL;



    return 1;
}

static inline void gfx_close(struct gfx *ctx) {
    if (ctx->backbuf) {
        free(ctx->backbuf);
        ctx->backbuf = NULL;
    }
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
    pit_delay_ms(ms);
}

/* ------------------------------------------------------------------ */
/*  Backbuffer helpers — transparent double-buffering for consumers    */
/* ------------------------------------------------------------------ */

/* Switch drawing target to the backbuffer, allocating it on first use */
static inline void gfx_backbuffer_begin(struct gfx *g) {
    if (!g->backbuf) {
        g->real_fb = g->fb;
        uint32_t sz = g->pitch * g->height;
        g->backbuf = malloc(sz);
        if (g->backbuf)
            memmove(g->backbuf, g->real_fb, sz);
    }
    g->fb = g->backbuf;
}

/* Blit backbuffer to the real framebuffer and restore fb pointer */
static inline void gfx_backbuffer_end(struct gfx *g) {
    if (g->backbuf && g->real_fb) {
        uint32_t sz = g->pitch * g->height;
        uint8_t *dst = g->real_fb;
        const uint8_t *src = (const uint8_t *)g->backbuf;

        /* Bulk copy via rep movsl (4× faster than movsb on WC framebuffer) */
        uint32_t dwords = sz >> 2;
        __asm__ volatile("rep movsl"
                         : "+S"(src), "+D"(dst), "+c"(dwords)
                         :
                         : "memory");
        uint32_t rem = sz & 3;
        if (rem)
            __asm__ volatile("rep movsb"
                             : "+S"(src), "+D"(dst), "+c"(rem)
                             :
                             : "memory");
        g->fb = g->real_fb;
    }
}

#endif /* BIOS_GFX_H */
