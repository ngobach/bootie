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
  /* Canvas — native pixel format, malloc'd once */
  uint8_t *fb;
  uint32_t width;       /* canvas width  (= CANVAS_W) */
  uint32_t height;      /* canvas height (= CANVAS_H) */
  uint32_t pitch;       /* bytes per scanline */
  uint8_t  bpp;         /* bytes per pixel (always 4 for GOP) */
  uint8_t  rshift, gshift, bshift;  /* channel shifts (canvas = native) */

  /* Hardware screen info (for flush centering) */
  uint32_t hw_width, hw_height;

  /* Screen sprite — RGBA pixel buffer for drawing */
  struct gfx_sprite screen;

  /* UEFI */
  void *gop;
  uint32_t saved_mode;

  /* Input */
  int has_key;
  int buffered_key;
};

static inline uint32_t gfx_width(const struct gfx *g) { return g->width; }
static inline uint32_t gfx_height(const struct gfx *g) { return g->height; }




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

  ctx->saved_mode = gop->Mode->Mode;

  /* mode selection */
  {
    typedef efi_status_t (EFIAPI *qm_t)(
        efi_graphics_output_protocol_t *, uint32_t,
        grub_size_t *, EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **);
    typedef efi_status_t (EFIAPI *sm_t)(
        efi_graphics_output_protocol_t *, uint32_t);

    qm_t qm = (qm_t)gop->QueryMode;
    sm_t sm = (sm_t)gop->SetMode;

    uint32_t nmax = gop->Mode->MaxMode;
    if (nmax > 64) nmax = 64;

    int      best_score = -1;
    uint32_t best_mode  = 0;

    for (uint32_t i = 0; i < nmax; i++) {
      grub_size_t sz = 0;
      EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *mi = NULL;
      if (qm(gop, i, &sz, &mi) != 0 || !mi)
        continue;
      if (mi->PixelFormat == PixelBltOnly || mi->PixelFormat == PixelFormatMax)
        continue;
      if (mi->HorizontalResolution < 800)
        continue;

      int s = 0;
      if      (mi->PixelFormat == PixelRedGreenBlueReserved8BitPerColor ||
               mi->PixelFormat == PixelBlueGreenRedReserved8BitPerColor)
        s = 100;
      else if (mi->PixelFormat == PixelBitMask)
        s = 90;

      if (mi->HorizontalResolution == 800 && mi->VerticalResolution == 600)
        s += 60;
      else if (mi->HorizontalResolution == 1024 && mi->VerticalResolution == 768)
        s += 30;
      else if (mi->HorizontalResolution == 1920 && mi->VerticalResolution == 1080)
        s += 20;

      if (s > best_score) {
        best_score = s;
        best_mode  = i;
      }
    }

    if (best_score >= 0 && best_mode != gop->Mode->Mode) {
      sm(gop, best_mode);
    }
  }

  /* Hardware screen info */
  ctx->hw_width  = gop->Mode->Info->HorizontalResolution;
  ctx->hw_height = gop->Mode->Info->VerticalResolution;

  /* Canvas pixel format (GOP is always 32-bit) */
  ctx->bpp = 4;
  ctx->pitch = CANVAS_W * 4;

  /* Determine channel shifts */
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
    while (temp && !(temp & 1)) { count++; temp >>= 1; }
    ctx->rshift = count;

    count = 0;
    temp = gmask;
    while (temp && !(temp & 1)) { count++; temp >>= 1; }
    ctx->gshift = count;

    count = 0;
    temp = bmask;
    while (temp && !(temp & 1)) { count++; temp >>= 1; }
    ctx->bshift = count;
  }

  /* Allocate one canvas in native format */
  ctx->width  = CANVAS_W;
  ctx->height = CANVAS_H;
  ctx->fb = (uint8_t *)malloc(ctx->pitch * ctx->height);
  if (!ctx->fb)
    return 0;
  /* Clear to black */
  {
    uint32_t *p = (uint32_t *)ctx->fb;
    uint32_t n = (ctx->pitch * ctx->height) / 4;
    __asm__ __volatile__("rep stosl" : "+D"(p), "+c"(n) : "a"(0) : "memory");
  }

  gfx_sprite_init(&ctx->screen, ctx->width, ctx->height);

  ctx->gop = gop;
  ctx->has_key = 0;
  ctx->buffered_key = 0;

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
  gfx_sprite_destroy(&ctx->screen);
  gfx_font_unload();
  if (ctx->gop) {
    efi_graphics_output_protocol_t *gop =
        (efi_graphics_output_protocol_t *)ctx->gop;
    if (gop->Mode->Mode != ctx->saved_mode) {
      typedef efi_status_t (EFIAPI *sm_t)(
          efi_graphics_output_protocol_t *, uint32_t);
      sm_t sm = (sm_t)gop->SetMode;
      sm(gop, ctx->saved_mode);
    }
  }
  if (ctx->fb) {
    free(ctx->fb);
    ctx->fb = NULL;
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
  if (status == 0) {
    efi_input_key_t key;
    status = st->con_in->read_key_stroke(st->con_in, &key);
    if (status == 0) {
      int code = 0;
      if (key.scan_code == 0) {
        code = key.unicode_char;
      } else {
        if (key.scan_code == 1)
          code = 0x4800;
        else if (key.scan_code == 2)
          code = 0x5000;
        else if (key.scan_code == 3)
          code = 0x4D00;
        else if (key.scan_code == 4)
          code = 0x4B00;
        else if (key.scan_code == 23)
          code = 27;
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

#endif /* UEFI_GFX_H */
