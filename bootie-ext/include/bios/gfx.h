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
  /* Canvas — native pixel format, malloc'd once */
  uint8_t *fb;
  uint32_t width;       /* canvas width  (= CANVAS_W) */
  uint32_t height;      /* canvas height (= CANVAS_H) */
  uint32_t pitch;       /* bytes per scanline */
  uint8_t  bpp;         /* bytes per pixel (2, 3, or 4) */
  uint8_t  rshift, gshift, bshift;

  /* Hardware screen info (for flush centering) */
  uint8_t *hw_fb;       /* physical framebuffer (not malloc'd) */
  uint32_t hw_width, hw_height, hw_pitch;

  uint16_t prev_vbe_mode;
};

static inline uint32_t gfx_width(const struct gfx *g) { return g->width; }
static inline uint32_t gfx_height(const struct gfx *g) { return g->height; }


/* ------------------------------------------------------------------ */
/*  Low-level BIOS helpers                                              */
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
    if (!(mi->ModeAttributes & 1))    return 0;
    if (!(mi->ModeAttributes & 0x80)) return 0;
    if (mi->PhysBasePtr == 0)         return 0;
    memmove(mi_out, mi, sizeof(*mi));
    return 1;
}

/* ------------------------------------------------------------------ */
/*  Framebuffer pixel writer (native format)                           */
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
        uint16_t c16 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
        *((uint16_t *)p) = c16;
    }
}

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
    {
        char find_buf[1024];
        bt_eval("echo", find_buf, sizeof(find_buf));
    }

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
        if (mi.MemoryModel != 4 && mi.MemoryModel != 6) continue;

        int score = 0;
        if      (mi.BitsPerPixel == 32) score += 100;
        else if (mi.BitsPerPixel == 24) score +=  90;
        else if (mi.BitsPerPixel == 16) score +=  50;
        else continue;

        if (mi.XResolution < 800) continue;

        score += 1000;
        if      (mi.XResolution ==  800 && mi.YResolution == 600)  score += 50;
        else if (mi.XResolution == 1024 && mi.YResolution == 768)  score += 40;
        else if (mi.XResolution == 1920 && mi.YResolution == 1080) score += 30;
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

    /* Save current VBE mode */
    ctx->prev_vbe_mode = 0;
    {
        struct realmode_regs rr = {
            0, 0, 0, (unsigned long)-1,
            0, 0, 0, 0x4F03,
            (unsigned long)-1, (unsigned long)-1,
            (unsigned long)-1, (unsigned long)-1,
            (unsigned long)-1, 0xFFFF10CD,
            (unsigned long)-1, (unsigned long)-1
        };
        realmode_run((long)&rr);
        if ((rr.eax & 0xFF) == 0x4F)
            ctx->prev_vbe_mode = (uint16_t)(rr.ebx & 0xFFFF);
    }

    printf("Setting VBE mode 0x%X (%dx%d, %d bpp)\n",
           (int)best_mode,
           (int)best_mi.XResolution, (int)best_mi.YResolution,
           (int)best_mi.BitsPerPixel);

    if ((bios_int10(0x4F02, 0x4000 | best_mode, 0, 0, (unsigned long)-1, 0) & 0xFF) != 0x4F) {
        printf("Failed to set VBE mode\n");
        return 0;
    }

    /* Hardware screen info */
    ctx->hw_fb    = (uint8_t *)best_mi.PhysBasePtr;
    ctx->hw_width = best_mi.XResolution;
    ctx->hw_height = best_mi.YResolution;
    ctx->hw_pitch = best_mi.LinBytesPerScanline ? best_mi.LinBytesPerScanline
                                              : best_mi.BytesPerScanline;

    /* Canvas pixel format */
    ctx->bpp = (best_mi.BitsPerPixel + 7) / 8;
    ctx->rshift = best_mi.LinRedFieldPosition   ? best_mi.LinRedFieldPosition
                                               : best_mi.RedFieldPosition;
    ctx->gshift = best_mi.LinGreenFieldPosition ? best_mi.LinGreenFieldPosition
                                               : best_mi.GreenFieldPosition;
    ctx->bshift = best_mi.LinBlueFieldPosition  ? best_mi.LinBlueFieldPosition
                                               : best_mi.BlueFieldPosition;

    /* Allocate one canvas in native format */
    ctx->width  = CANVAS_W;
    ctx->height = CANVAS_H;
    ctx->pitch  = CANVAS_W * ctx->bpp;
    ctx->fb = (uint8_t *)malloc(ctx->pitch * ctx->height);
    if (!ctx->fb)
        return 0;
    {
        uint32_t *p = (uint32_t *)ctx->fb;
        uint32_t n = (ctx->pitch * ctx->height) / 4;
        __asm__ __volatile__("rep stosl" : "+D"(p), "+c"(n) : "a"(0) : "memory");
    }

    if (gfx_font_load() < 0) {
        if (ctx->fb) {
            free(ctx->fb);
            ctx->fb = NULL;
        }
        return 0;
    }
    return 1;
}

static inline void gfx_close(struct gfx *ctx) {
    gfx_font_unload();
    if (ctx->fb) {
        free(ctx->fb);
        ctx->fb = NULL;
    }
    if (ctx->prev_vbe_mode) {
        bios_int10(0x4F02, ctx->prev_vbe_mode, 0, 0, (unsigned long)-1, 0);
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
/*  Flush canvas to hardware screen                                   */
/*  Canvas is native format — copy row by row, centered.              */
/* ------------------------------------------------------------------ */
static inline void gfx_flush(struct gfx *g) {
    if (!g->fb || !g->hw_fb)
        return;

    int ox = ((int)g->hw_width  - (int)g->width)  / 2;
    int oy = ((int)g->hw_height - (int)g->height) / 2;
    if (ox < 0) ox = 0;
    if (oy < 0) oy = 0;

    uint32_t row_bytes = g->width * g->bpp;

    for (uint32_t y = 0; y < g->height; y++) {
        uint8_t *src = g->fb + y * g->pitch;
        uint8_t *dst = g->hw_fb + (unsigned)(oy + (int)y) * g->hw_pitch
                     + (unsigned)ox * g->bpp;

        /* Bulk copy via rep movsl */
        uint32_t dwords = row_bytes >> 2;
        __asm__ volatile("rep movsl"
                         : "+S"(src), "+D"(dst), "+c"(dwords)
                         :
                         : "memory");
        uint32_t rem = row_bytes & 3;
        if (rem)
            __asm__ volatile("rep movsb"
                             : "+S"(src), "+D"(dst), "+c"(rem)
                             :
                             : "memory");
    }
}

#endif /* BIOS_GFX_H */
