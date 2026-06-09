#ifndef UEFI_GFX_H
#define UEFI_GFX_H

#include <bootie.h>
#include <stdint.h>

/* ------------------------------------------------------------------ */
/*  UEFI GOP structures                                               */
/* ------------------------------------------------------------------ */

typedef enum {
  PixelRedGreenBlueReserved8BitPerColor,
  PixelBlueGreenRedReserved8BitPerColor,
  PixelBitMask,
  PixelBltOnly,
  PixelFormatMax
} EFI_GRAPHICS_OUTPUT_RENDER_EFFECT;

typedef struct {
  uint32_t RedMask;
  uint32_t GreenMask;
  uint32_t BlueMask;
  uint32_t ReservedMask;
} EFI_PIXEL_BITMASK;

typedef struct {
  uint32_t Version;
  uint32_t HorizontalResolution;
  uint32_t VerticalResolution;
  EFI_GRAPHICS_OUTPUT_RENDER_EFFECT PixelFormat;
  EFI_PIXEL_BITMASK PixelInformation;
  uint32_t PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct {
  uint32_t MaxMode;
  uint32_t Mode;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
  grub_size_t SizeOfInfo;
  efi_physical_address_t FrameBufferBase;
  grub_size_t FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

typedef struct efi_graphics_output_protocol {
  void *QueryMode;
  void *SetMode;
  void *Blt;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *Mode;
} efi_graphics_output_protocol_t;

/* ------------------------------------------------------------------ */
/*  Graphics Context Structure                                        */
/* ------------------------------------------------------------------ */
struct gfx {
  uint8_t *fb;
  uint32_t width;
  uint32_t height;
  uint32_t pitch;
  uint32_t bpp;
  uint8_t rshift;
  uint8_t gshift;
  uint8_t bshift;
  int has_key;
  int buffered_key;
  uint8_t *real_fb;
  void *backbuf;
  void *gop;
};

static inline uint32_t gfx_width(const struct gfx *g) { return g->width; }

static inline uint32_t gfx_height(const struct gfx *g) { return g->height; }

/* ------------------------------------------------------------------ */
/*  5x7 bitmap font (printable ASCII 0x20..0x7E)                       */
/* ------------------------------------------------------------------ */
static const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, /* ' ' */
    {0x00, 0x00, 0x5F, 0x00, 0x00}, /* '!' */
    {0x00, 0x07, 0x00, 0x07, 0x00}, /* '"' */
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, /* '#' */
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, /* '$' */
    {0x23, 0x13, 0x08, 0x64, 0x62}, /* '%' */
    {0x36, 0x49, 0x55, 0x22, 0x50}, /* '&' */
    {0x00, 0x05, 0x03, 0x00, 0x00}, /* '\'' */
    {0x00, 0x1C, 0x22, 0x41, 0x00}, /* '(' */
    {0x00, 0x41, 0x22, 0x1C, 0x00}, /* ')' */
    {0x14, 0x08, 0x3E, 0x08, 0x14}, /* '*' */
    {0x08, 0x08, 0x3E, 0x08, 0x08}, /* '+' */
    {0x00, 0x50, 0x30, 0x00, 0x00}, /* ',' */
    {0x08, 0x08, 0x08, 0x08, 0x08}, /* '-' */
    {0x00, 0x60, 0x60, 0x00, 0x00}, /* '.' */
    {0x20, 0x10, 0x08, 0x04, 0x02}, /* '/' */
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, /* '0' */
    {0x00, 0x42, 0x7F, 0x40, 0x00}, /* '1' */
    {0x42, 0x61, 0x51, 0x49, 0x46}, /* '2' */
    {0x21, 0x41, 0x45, 0x4B, 0x31}, /* '3' */
    {0x18, 0x14, 0x12, 0x7F, 0x10}, /* '4' */
    {0x27, 0x45, 0x45, 0x45, 0x39}, /* '5' */
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, /* '6' */
    {0x01, 0x71, 0x09, 0x05, 0x03}, /* '7' */
    {0x36, 0x49, 0x49, 0x49, 0x36}, /* '8' */
    {0x06, 0x49, 0x49, 0x29, 0x1E}, /* '9' */
    {0x00, 0x36, 0x36, 0x00, 0x00}, /* ':' */
    {0x00, 0x56, 0x36, 0x00, 0x00}, /* ';' */
    {0x08, 0x14, 0x22, 0x41, 0x00}, /* '<' */
    {0x14, 0x14, 0x14, 0x14, 0x14}, /* '=' */
    {0x00, 0x41, 0x22, 0x14, 0x08}, /* '>' */
    {0x02, 0x01, 0x51, 0x09, 0x06}, /* '?' */
    {0x32, 0x49, 0x79, 0x41, 0x3E}, /* '@' */
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, /* 'A' */
    {0x7F, 0x49, 0x49, 0x49, 0x36}, /* 'B' */
    {0x3E, 0x41, 0x41, 0x41, 0x22}, /* 'C' */
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, /* 'D' */
    {0x7F, 0x49, 0x49, 0x49, 0x41}, /* 'E' */
    {0x7F, 0x09, 0x09, 0x09, 0x01}, /* 'F' */
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, /* 'G' */
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, /* 'H' */
    {0x00, 0x41, 0x7F, 0x41, 0x00}, /* 'I' */
    {0x20, 0x40, 0x41, 0x3F, 0x01}, /* 'J' */
    {0x7F, 0x08, 0x14, 0x22, 0x41}, /* 'K' */
    {0x7F, 0x40, 0x40, 0x40, 0x40}, /* 'L' */
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, /* 'M' */
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, /* 'N' */
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, /* 'O' */
    {0x7F, 0x09, 0x09, 0x09, 0x06}, /* 'P' */
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, /* 'Q' */
    {0x7F, 0x09, 0x19, 0x29, 0x46}, /* 'R' */
    {0x46, 0x49, 0x49, 0x49, 0x31}, /* 'S' */
    {0x01, 0x01, 0x7F, 0x01, 0x01}, /* 'T' */
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, /* 'U' */
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, /* 'V' */
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, /* 'W' */
    {0x63, 0x14, 0x08, 0x14, 0x63}, /* 'X' */
    {0x07, 0x08, 0x70, 0x08, 0x07}, /* 'Y' */
    {0x61, 0x51, 0x49, 0x45, 0x43}, /* 'Z' */
    {0x00, 0x7F, 0x41, 0x41, 0x00}, /* '[' */
    {0x02, 0x04, 0x08, 0x10, 0x20}, /* '\\' */
    {0x00, 0x41, 0x41, 0x7F, 0x00}, /* ']' */
    {0x04, 0x02, 0x01, 0x02, 0x04}, /* '^' */
    {0x40, 0x40, 0x40, 0x40, 0x40}, /* '_' */
    {0x00, 0x01, 0x02, 0x04, 0x00}, /* '`' */
    {0x20, 0x54, 0x54, 0x54, 0x78}, /* 'a' */
    {0x7F, 0x48, 0x44, 0x44, 0x38}, /* 'b' */
    {0x38, 0x44, 0x44, 0x44, 0x20}, /* 'c' */
    {0x38, 0x44, 0x44, 0x48, 0x7F}, /* 'd' */
    {0x38, 0x54, 0x54, 0x54, 0x18}, /* 'e' */
    {0x08, 0x7E, 0x09, 0x01, 0x02}, /* 'f' */
    {0x0C, 0x52, 0x52, 0x52, 0x3E}, /* 'g' */
    {0x7F, 0x08, 0x04, 0x04, 0x78}, /* 'h' */
    {0x00, 0x44, 0x7D, 0x40, 0x00}, /* 'i' */
    {0x20, 0x40, 0x44, 0x3D, 0x00}, /* 'j' */
    {0x7F, 0x10, 0x28, 0x44, 0x00}, /* 'k' */
    {0x00, 0x41, 0x7F, 0x40, 0x00}, /* 'l' */
    {0x7C, 0x04, 0x18, 0x04, 0x78}, /* 'm' */
    {0x7C, 0x08, 0x04, 0x04, 0x78}, /* 'n' */
    {0x38, 0x44, 0x44, 0x44, 0x38}, /* 'o' */
    {0x7C, 0x14, 0x14, 0x14, 0x08}, /* 'p' */
    {0x08, 0x14, 0x14, 0x18, 0x7C}, /* 'q' */
    {0x7C, 0x08, 0x04, 0x04, 0x08}, /* 'r' */
    {0x48, 0x54, 0x54, 0x54, 0x20}, /* 's' */
    {0x04, 0x3F, 0x44, 0x40, 0x20}, /* 't' */
    {0x3C, 0x40, 0x40, 0x40, 0x7C}, /* 'u' */
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, /* 'v' */
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, /* 'w' */
    {0x44, 0x28, 0x10, 0x28, 0x44}, /* 'x' */
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, /* 'y' */
    {0x44, 0x64, 0x54, 0x4C, 0x44}, /* 'z' */
    {0x00, 0x08, 0x36, 0x41, 0x00}, /* '{' */
    {0x00, 0x00, 0x7F, 0x00, 0x00}, /* '|' */
    {0x00, 0x41, 0x36, 0x08, 0x00}, /* '}' */
    {0x10, 0x08, 0x08, 0x10, 0x08}, /* '~' */
};

/* ------------------------------------------------------------------ */
/*  Framebuffer pixel writer                                            */
/* ------------------------------------------------------------------ */
static inline __attribute__((always_inline)) void
put_pixel(struct gfx *ctx, uint32_t x, uint32_t y, uint8_t r, uint8_t g,
          uint8_t b) {
  if (x >= ctx->width || y >= ctx->height)
    return;
  uint8_t *p = ctx->fb + y * ctx->pitch + x * ctx->bpp;
  uint32_t color = ((uint32_t)r << ctx->rshift) | ((uint32_t)g << ctx->gshift) |
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

static void fill_rect(struct gfx *ctx, uint32_t x, uint32_t y, uint32_t w,
                      uint32_t h, uint8_t r, uint8_t g, uint8_t b) {
  if (x >= ctx->width || y >= ctx->height)
    return;
  uint32_t x_end = (x + w > ctx->width) ? ctx->width : (x + w);
  uint32_t y_end = (y + h > ctx->height) ? ctx->height : (y + h);
  if (x_end <= x || y_end <= y)
    return;

  uint32_t color = ((uint32_t)r << ctx->rshift) | ((uint32_t)g << ctx->gshift) |
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

static void draw_char(struct gfx *ctx, uint32_t cx, uint32_t cy, char ch,
                      uint8_t r, uint8_t g, uint8_t b, uint32_t scale) {
  if (ch < 0x20 || ch > 0x7E)
    ch = '?';
  const uint8_t *glyph = font5x7[ch - 0x20];
  uint32_t color = ((uint32_t)r << ctx->rshift) | ((uint32_t)g << ctx->gshift) |
                   ((uint32_t)b << ctx->bshift);

  if (ctx->bpp == 4) {
    for (uint32_t col = 0; col < 5; col++) {
      for (uint32_t row = 0; row < 7; row++) {
        if (!(glyph[col] & (1 << row))) continue;
        uint32_t px = cx + col * scale;
        uint32_t py = cy + row * scale;
        for (uint32_t sy = 0; sy < scale; sy++) {
          uint32_t *dst = (uint32_t *)(ctx->fb + (py + sy) * ctx->pitch) + px;
          for (uint32_t sx = 0; sx < scale; sx++) dst[sx] = color;
        }
      }
    }
  } else if (ctx->bpp == 3) {
    uint8_t b0 = color & 0xFF;
    uint8_t b1 = (color >> 8) & 0xFF;
    uint8_t b2 = (color >> 16) & 0xFF;
    for (uint32_t col = 0; col < 5; col++) {
      for (uint32_t row = 0; row < 7; row++) {
        if (!(glyph[col] & (1 << row))) continue;
        uint32_t px = cx + col * scale;
        uint32_t py = cy + row * scale;
        for (uint32_t sy = 0; sy < scale; sy++) {
          uint8_t *dst = ctx->fb + (py + sy) * ctx->pitch + px * 3;
          for (uint32_t sx = 0; sx < scale; sx++) {
            dst[0] = b0; dst[1] = b1; dst[2] = b2;
            dst += 3;
          }
        }
      }
    }
  } else if (ctx->bpp == 2) {
    uint16_t c16 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
    for (uint32_t col = 0; col < 5; col++) {
      for (uint32_t row = 0; row < 7; row++) {
        if (!(glyph[col] & (1 << row))) continue;
        uint32_t px = cx + col * scale;
        uint32_t py = cy + row * scale;
        for (uint32_t sy = 0; sy < scale; sy++) {
          uint16_t *dst = (uint16_t *)(ctx->fb + (py + sy) * ctx->pitch) + px;
          for (uint32_t sx = 0; sx < scale; sx++) dst[sx] = c16;
        }
      }
    }
  }
}

static void draw_str(struct gfx *ctx, uint32_t cx, uint32_t cy, const char *s,
                     uint8_t r, uint8_t g, uint8_t b, uint32_t scale) {
  while (*s) {
    draw_char(ctx, cx, cy, *s, r, g, b, scale);
    cx += (5 + 1) * scale;
    s++;
  }
}

/* ------------------------------------------------------------------ */
/*  Platform Agnostic Wrappers                                        */
/* ------------------------------------------------------------------ */

static inline int gfx_init(struct gfx *ctx) {
  efi_guid_t gop_guid = {0x9042a9de,
                         0x23dc,
                         0x4a38,
                         {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}};
  efi_graphics_output_protocol_t *gop = NULL;
  efi_system_table_t *st = grub_efi_system_table;
  if (!st || !st->boot_services)
    return 0;

  efi_status_t status =
      st->boot_services->locate_protocol(&gop_guid, NULL, (void **)&gop);
  if (status != 0 || !gop || !gop->Mode || !gop->Mode->Info) {
    return 0;
  }

  ctx->fb = (uint8_t *)gop->Mode->FrameBufferBase;
  ctx->width = gop->Mode->Info->HorizontalResolution;
  ctx->height = gop->Mode->Info->VerticalResolution;
  ctx->bpp = 4; /* GOP is always 32-bit linear framebuffer */
  ctx->pitch = gop->Mode->Info->PixelsPerScanLine * 4;

  /* Determine shifts */
  ctx->rshift = 16;
  ctx->gshift = 8;
  ctx->bshift = 0;

  if (gop->Mode->Info->PixelFormat == PixelRedGreenBlueReserved8BitPerColor) {
    ctx->rshift = 0;
    ctx->gshift = 8;
    ctx->bshift = 16;
  } else if (gop->Mode->Info->PixelFormat == PixelBitMask) {
    uint32_t rmask = gop->Mode->Info->PixelInformation.RedMask;
    uint32_t gmask = gop->Mode->Info->PixelInformation.GreenMask;
    uint32_t bmask = gop->Mode->Info->PixelInformation.BlueMask;

    uint8_t count = 0;
    uint32_t temp = rmask;
    while (temp && !(temp & 1)) {
      count++;
      temp >>= 1;
    }
    ctx->rshift = count;

    count = 0;
    temp = gmask;
    while (temp && !(temp & 1)) {
      count++;
      temp >>= 1;
    }
    ctx->gshift = count;

    count = 0;
    temp = bmask;
    while (temp && !(temp & 1)) {
      count++;
      temp >>= 1;
    }
    ctx->bshift = count;
  }

  ctx->has_key = 0;
  ctx->buffered_key = 0;
  ctx->real_fb = NULL;
  ctx->backbuf = NULL;
  ctx->gop = gop;
  return 1;
}

static inline void gfx_close(struct gfx *ctx) {
  if (ctx->backbuf) {
    free(ctx->backbuf);
    ctx->backbuf = NULL;
  }
  if (graphics_inited) {
    if (current_term->STARTUP)
      ((int (*)(int))current_term->STARTUP)(0);
  }
  cls();
}

static inline void gfx_delay_ms(struct gfx *ctx, unsigned int ms) {
  (void)ctx;
  efi_system_table_t *st = grub_efi_system_table;
  if (st && st->boot_services) {
    st->boot_services->stall(ms * 1000);
  }
}

static inline int gfx_checkkey(struct gfx *ctx) {
  if (ctx->has_key)
    return 1;

  efi_system_table_t *st = grub_efi_system_table;
  if (!st || !st->con_in || !st->boot_services)
    return 0;

  efi_status_t status = st->boot_services->check_event(st->con_in->wait_for_key);
  if (status == 0) { // EFI_SUCCESS (event is signaled, key is ready)
    efi_input_key_t key;
    status = st->con_in->read_key_stroke(st->con_in, &key);
    if (status == 0) {
      int code = 0;
      if (key.scan_code == 0) {
        code = key.unicode_char;
      } else {
        // Map UEFI scan codes to BIOS codes expected by the game
        if (key.scan_code == 1)
          code = 0x4800; // Up
        else if (key.scan_code == 2)
          code = 0x5000; // Down
        else if (key.scan_code == 3)
          code = 0x4D00; // Right
        else if (key.scan_code == 4)
          code = 0x4B00; // Left
        else if (key.scan_code == 23)
          code = 27; // ESC
        else
          code = key.scan_code << 8;
      }
      ctx->buffered_key = code;
      ctx->has_key = 1;
      return 1;
    }
  }
  return 0;
}

static inline int gfx_getkey(struct gfx *ctx) {
  while (!gfx_checkkey(ctx)) {
    gfx_delay_ms(ctx, 10);
  }
  ctx->has_key = 0;
  return ctx->buffered_key;
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
    if (g->backbuf) {
      /* Preserve current screen into backbuffer (one-time copy).
         Use GOP Blt (EfiBltVideoToBltBuffer) for WC framebuffer if
         pixel format matches BLT_PIXEL (BGRx). */
      int copied = 0;
      if (g->gop && g->bshift == 0 && g->gshift == 8 && g->rshift == 16) {
        efi_graphics_output_protocol_t *gop =
            (efi_graphics_output_protocol_t *)g->gop;
        if (gop->Blt) {
          typedef efi_status_t(EFIAPI *blt_t)(void *, void *, uint32_t,
                                               uint32_t, uint32_t, uint32_t,
                                               uint32_t, uint32_t, uint32_t,
                                               uint32_t);
          if (((blt_t)gop->Blt)(gop, g->backbuf, 1 /*EfiBltVideoToBltBuffer*/,
                                0, 0, 0, 0, g->width, g->height, g->pitch) == 0)
            copied = 1;
        }
      }
      if (!copied) {
        uint8_t *dst = g->backbuf;
        const uint8_t *src = g->real_fb;
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
      }
    }
  }
  g->fb = g->backbuf;
}

/* Blit backbuffer to the real framebuffer and restore fb pointer */
static inline void gfx_backbuffer_end(struct gfx *g) {
  if (!g->backbuf || !g->real_fb)
    return;

  /* Prefer GOP Blt when pixel layout matches BLT_PIXEL (BGRx) */
  if (g->gop && g->bshift == 0 && g->gshift == 8 && g->rshift == 16) {
    efi_graphics_output_protocol_t *gop =
        (efi_graphics_output_protocol_t *)g->gop;
    if (gop->Blt) {
      typedef efi_status_t(EFIAPI *blt_t)(void *, void *, uint32_t,
                                           uint32_t, uint32_t, uint32_t,
                                           uint32_t, uint32_t, uint32_t,
                                           uint32_t);
      ((blt_t)gop->Blt)(gop, g->backbuf, 2 /*EfiBltBufferToVideo*/,
                        0, 0, 0, 0, g->width, g->height, g->pitch);
      g->fb = g->real_fb;
      return;
    }
  }

  /* CPU fallback (only when ENABLE_UEFI_SLOW_BLIT is defined) */
#ifdef ENABLE_UEFI_SLOW_BLIT
  {
    uint32_t sz = g->pitch * g->height;
    uint8_t *dst = g->real_fb;
    const uint8_t *src = (const uint8_t *)g->backbuf;
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
  }
#endif
  g->fb = g->real_fb;
}

#endif /* UEFI_GFX_H */
