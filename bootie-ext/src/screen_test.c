#include <bootprog.h>

#if !defined(__i386__)

typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;

#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID                                      \
  {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}}

typedef enum {
  PixelRedGreenBlueReserved8BitPerColor,
  PixelBlueGreenRedReserved8BitPerColor,
  PixelBitMask,
  PixelBltOnly,
  PixelFormatMax
} EFI_GRAPHICS_OUTPUT_OBJECT_PIXEL_FORMAT;

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
  EFI_GRAPHICS_OUTPUT_OBJECT_PIXEL_FORMAT PixelFormat;
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

typedef struct _EFI_GRAPHICS_OUTPUT_PROTOCOL EFI_GRAPHICS_OUTPUT_PROTOCOL;

struct _EFI_GRAPHICS_OUTPUT_PROTOCOL {
  void *QueryMode;
  void *SetMode;
  void *Blt;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *Mode;
};

int main(char *arg, int flags) {
  INIT_BOOT_MODULE();

  printf("UEFI screen_test POC started.\n");

  efi_system_table_t *st = (efi_system_table_t *)grub_efi_system_table;
  printf("SystemTable: 0x%lx\n", (unsigned long)st);
  if (!st) {
    printf("st is NULL!\n");
    return 0;
  }
  printf("BootServices: 0x%lx\n", (unsigned long)st->boot_services);
  if (!st->boot_services) {
    printf("boot_services is NULL!\n");
    return 0;
  }

  efi_guid_t gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;

  efi_status_t status =
      st->boot_services->locate_protocol(&gop_guid, NULL, (void **)&gop);
  printf("locate_protocol status: 0x%lx, GOP ptr: 0x%lx\n",
         (unsigned long)status, (unsigned long)gop);

  if (status == EFI_SUCCESS && gop) {
    // Dump GOP structure bytes
    printf("GOP struct memory: ");
    unsigned char *gop_bytes = (unsigned char *)gop;
    for (int i = 0; i < 32; i++) {
      printf("%02x ", gop_bytes[i]);
    }
    printf("\n");

    printf("GOP Mode: 0x%lx\n", (unsigned long)gop->Mode);
    if (gop->Mode) {
      // Dump GOP Mode structure bytes
      printf("GOP Mode struct memory: ");
      unsigned char *mode_bytes = (unsigned char *)gop->Mode;
      for (int i = 0; i < 48; i++) {
        printf("%02x ", mode_bytes[i]);
      }
      printf("\n");

      printf("FrameBufferBase: 0x%lx\n",
             (unsigned long)gop->Mode->FrameBufferBase);
      printf("FrameBufferSize: 0x%lx\n",
             (unsigned long)gop->Mode->FrameBufferSize);
      if (gop->Mode->Info) {
        printf("Resolution: %dx%d, Stride: %d, Format: %d\n",
               (int)gop->Mode->Info->HorizontalResolution,
               (int)gop->Mode->Info->VerticalResolution,
               (int)gop->Mode->Info->PixelsPerScanLine,
               (int)gop->Mode->Info->PixelFormat);
      }

      if (gop->Mode->FrameBufferBase) {
        uint32_t *fb = (uint32_t *)(grub_size_t)gop->Mode->FrameBufferBase;
        uint32_t width = gop->Mode->Info->HorizontalResolution;
        uint32_t height = gop->Mode->Info->VerticalResolution;
        uint32_t stride = gop->Mode->Info->PixelsPerScanLine;

        uint32_t color = 0x000000FF; // BGRX Blue
        if (gop->Mode->Info->PixelFormat ==
            PixelRedGreenBlueReserved8BitPerColor) {
          color = 0x00FF0000; // RGBX Blue
        }

        printf("Drawing blue screen with color 0x%x...\n",
               (unsigned int)color);
        for (uint32_t y = 0; y < height; y++) {
          uint32_t *line = fb + (y * stride);
          for (uint32_t x = 0; x < width; x++) {
            line[x] = color;
          }
        }
        printf("Drawing finished.\n");
      }
    }
  } else {
    printf("Failed to locate GOP protocol!\n");
  }

  printf("POC finished. Press any key to exit...\n");
  while (!checkkey()) {
    // wait
  }
  getkey();

  return 0;
}
#else

typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;

struct vbe_mode_info {
  uint16_t ModeAttributes;
  uint8_t WinAAttributes;
  uint8_t WinBAttributes;
  uint16_t WinGranularity;
  uint16_t WinSize;
  uint16_t WinASegment;
  uint16_t WinBSegment;
  uint32_t WinFuncPtr;
  uint16_t BytesPerScanline;

  // VBE 1.2+
  uint16_t XResolution;
  uint16_t YResolution;
  uint8_t XCharSize;
  uint8_t YCharSize;
  uint8_t NumberOfPlanes;
  uint8_t BitsPerPixel;
  uint8_t NumberOfBanks;
  uint8_t MemoryModel;
  uint8_t BankSize;
  uint8_t NumberOfImagePages;
  uint8_t Reserved1;

  // VBE 1.2+ Direct Color fields
  uint8_t RedMaskSize;
  uint8_t RedFieldPosition;
  uint8_t GreenMaskSize;
  uint8_t GreenFieldPosition;
  uint8_t BlueMaskSize;
  uint8_t BlueFieldPosition;
  uint8_t RsvdMaskSize;
  uint8_t RsvdFieldPosition;
  uint8_t DirectColorModeInfo;

  // VBE 2.0+
  uint32_t PhysBasePtr;
  uint32_t Reserved2;
  uint16_t Reserved3;

  // VBE 3.0+
  uint16_t LinBytesPerScanline;
  uint8_t BnkNumberOfImagePages;
  uint8_t LinNumberOfImagePages;
  uint8_t LinRedMaskSize;
  uint8_t LinRedFieldPosition;
  uint8_t LinGreenMaskSize;
  uint8_t LinGreenFieldPosition;
  uint8_t LinBlueMaskSize;
  uint8_t LinBlueFieldPosition;
  uint8_t LinRsvdMaskSize;
  uint8_t LinRsvdFieldPosition;
  uint32_t MaxPixelClock;

  uint8_t Reserved4[189];
} __attribute__((packed));

struct vbe_driver_info {
  uint8_t VBESignature[4];
  uint16_t VBEVersion;
  uint32_t OEMStringPtr;
  uint32_t Capabilities;
  uint32_t VideoModePtr;
  uint16_t TotalMemory;

  // VBE 2.0 extensions
  uint16_t OemSoftwareRev;
  uint32_t OemVendorNamePtr;
  uint32_t OemProductNamePtr;
  uint32_t OemProductRevPtr;
  uint8_t Reserved[222];
  uint8_t OemDATA[256];
} __attribute__((packed));

struct realmode_regs {
  unsigned long edi;    // as input and output
  unsigned long esi;    // as input and output
  unsigned long ebp;    // as input and output
  unsigned long esp;    // stack pointer, as input
  unsigned long ebx;    // as input and output
  unsigned long edx;    // as input and output
  unsigned long ecx;    // as input and output
  unsigned long eax;    // as input and output
  unsigned long gs;     // as input and output
  unsigned long fs;     // as input and output
  unsigned long es;     // as input and output
  unsigned long ds;     // as input and output
  unsigned long ss;     // stack segment, as input
  unsigned long eip;    // instruction pointer, as input
  unsigned long cs;     // code segment, as input
  unsigned long eflags; // as input and output
};

struct test_color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  const char *name;
};

// Forward declarations - main must be the very first function defined
static void delay_ms(unsigned int ms);
static int bios_int10(unsigned long eax, unsigned long ebx, unsigned long ecx,
                      unsigned long edx, unsigned long es, unsigned long edi);
static uint32_t sys_RM16ToFlat32(uint32_t p_RMSegOfs);
static int get_driver_info(struct vbe_driver_info *drv);
static int get_mode_info(uint16_t mode, struct vbe_mode_info *modeInfo);
static uint32_t get_color_val(uint8_t r, uint8_t g, uint8_t b,
                              struct vbe_mode_info *mi);
static void fill_screen(uint32_t color_val, struct vbe_mode_info *mi);
static int bios_checkkey(void);
static int bios_getkey(void);

int main(char *arg, int flags) {
  INIT_BOOT_MODULE();

  struct vbe_driver_info drv;
  if (!get_driver_info(&drv)) {
    printf("VBE driver not supported.\n");
    return 0;
  }

  uint16_t *modes = (uint16_t *)drv.VideoModePtr;
  uint16_t best_mode = 0;
  int best_score = -1;
  struct vbe_mode_info best_mi;

  for (int idx = 0; modes[idx] != 0xFFFF; idx++) {
    uint16_t mode = modes[idx];
    struct vbe_mode_info temp_mi;
    if (!get_mode_info(mode, &temp_mi)) {
      continue;
    }

    // Score based on BitsPerPixel and Resolution
    int score = 0;
    if (temp_mi.BitsPerPixel == 32)
      score += 100;
    else if (temp_mi.BitsPerPixel == 24)
      score += 90;
    else if (temp_mi.BitsPerPixel == 16)
      score += 50;
    else if (temp_mi.BitsPerPixel == 15)
      score += 40;
    else
      continue; // skip 8-bit or lower modes

    if (temp_mi.XResolution == 800 && temp_mi.YResolution == 600)
      score += 50;
    else if (temp_mi.XResolution == 1024 && temp_mi.YResolution == 768)
      score += 40;
    else if (temp_mi.XResolution == 640 && temp_mi.YResolution == 480)
      score += 30;
    else
      score += 10;

    if (score > best_score) {
      best_score = score;
      best_mode = mode;
      best_mi = temp_mi;
    }
  }

  if (best_mode == 0) {
    printf("No suitable VBE graphics mode found.\n");
    return 0;
  }

  // Set selected mode (0x4000 flag requesting Linear Frame Buffer)
  if (bios_int10(0x4F02, 0x4000 | best_mode, 0, 0, -1, 0) != 0x004F) {
    printf("Failed to set VBE mode 0x%X.\n", best_mode);
    return 0;
  }

  struct test_color colors[] = {{255, 0, 0, "Red"},
                                {0, 255, 0, "Green"},
                                {0, 0, 255, "Blue"},
                                {0, 0, 0, "Black"},
                                {255, 255, 255, "White"}};

  int exit_early = 0;

  for (int i = 0; i < 5; i++) {
    uint32_t color_val =
        get_color_val(colors[i].r, colors[i].g, colors[i].b, &best_mi);
    fill_screen(color_val, &best_mi);

    unsigned int elapsed = 0;
    while (elapsed < 1000) {
      delay_ms(50);
      elapsed += 50;
      if (bios_checkkey()) {
        int key = bios_getkey();
        if (key == 27 || key == 0x1b) {
          exit_early = 1;
        }
        break; // skip to next color or exit
      }
    }

    if (exit_early) {
      break;
    }
  }

  // Restore original video state
  if (graphics_inited) {
    if (current_term->STARTUP) {
      ((int (*)(int))current_term->STARTUP)(0);
    }
  } else {
    bios_int10(3, 0, 0, 0, -1, 0);
  }
  cls();

  return 0;
}

static void delay_ms(unsigned int ms) {
  // 1 tick = 55 milliseconds on BIOS system timer
  unsigned int ticks = (ms + 54) / 55;
  unsigned long start = *(volatile unsigned long *)0x46c;
  while ((*(volatile unsigned long *)0x46c - start) < ticks) {
    // busy wait
  }
}

static int bios_int10(unsigned long eax, unsigned long ebx, unsigned long ecx,
                      unsigned long edx, unsigned long es, unsigned long edi) {
  struct realmode_regs int_regs = {edi, 0,  0,  -1, ebx, edx,        ecx, eax,
                                   -1,  -1, es, -1, -1,  0xFFFF10CD, -1,  -1};
  realmode_run((long)&int_regs);
  return (int)int_regs.eax;
}

static uint32_t sys_RM16ToFlat32(uint32_t p_RMSegOfs) {
  uint32_t hi, lo;
  hi = (p_RMSegOfs >> 16);
  lo = (uint16_t)p_RMSegOfs;
  return (hi << 4) + lo;
}

static int get_driver_info(struct vbe_driver_info *drv) {
  char vbe2Sig[4] = "VBE2";
  struct vbe_driver_info *di = (struct vbe_driver_info *)0x20000;
  memset(di, 0, sizeof(struct vbe_driver_info));
  memmove(di->VBESignature, vbe2Sig, 4);

  if ((bios_int10(0x4F00, 0, 0, 0, 0x2000, 0) == 0x4f) &&
      (memcmp(di->VBESignature, "VESA", 4) == 0) && (di->VBEVersion >= 0x200)) {
    di->VideoModePtr = sys_RM16ToFlat32(di->VideoModePtr);
    memmove(drv, di, sizeof(struct vbe_driver_info));
    return 1;
  }
  return 0;
}

static int get_mode_info(uint16_t mode, struct vbe_mode_info *modeInfo) {
  if (!mode || mode == 0xFFFF)
    return 0;

  struct vbe_mode_info *mi = (struct vbe_mode_info *)0x20400;
  memset(mi, 0, sizeof(struct vbe_mode_info));

  if (bios_int10(0x4F01, 0, mode, 0, 0x2000, 1024) != 0x004F)
    return 0;

  if ((mi->ModeAttributes & 1) == 0) // Mode supported in hardware
    return 0;
  if ((mi->ModeAttributes & 0x80) == 0) // LFB must be present
    // Note: Bit 7 of ModeAttributes indicates Linear Frame Buffer availability.
    return 0;
  if (mi->PhysBasePtr == 0)
    return 0;

  memmove(modeInfo, mi, sizeof(struct vbe_mode_info));
  return 1;
}

static uint32_t get_color_val(uint8_t r, uint8_t g, uint8_t b,
                              struct vbe_mode_info *mi) {
  uint32_t val = 0;
  uint32_t r_val = (r >> (8 - mi->RedMaskSize)) << mi->RedFieldPosition;
  uint32_t g_val = (g >> (8 - mi->GreenMaskSize)) << mi->GreenFieldPosition;
  uint32_t b_val = (b >> (8 - mi->BlueMaskSize)) << mi->BlueFieldPosition;
  val = r_val | g_val | b_val;
  return val;
}

static void fill_screen(uint32_t color_val, struct vbe_mode_info *mi) {
  uint8_t *fb = (uint8_t *)mi->PhysBasePtr;
  uint32_t width = mi->XResolution;
  uint32_t height = mi->YResolution;
  uint32_t bytes_per_line = mi->BytesPerScanline;
  uint8_t bpp = (mi->BitsPerPixel + 7) / 8;

  if (bpp == 4) {
    for (uint32_t y = 0; y < height; y++) {
      uint32_t *line = (uint32_t *)(fb + y * bytes_per_line);
      for (uint32_t x = 0; x < width; x++) {
        line[x] = color_val;
      }
    }
  } else if (bpp == 3) {
    uint8_t c0 = color_val & 0xFF;
    uint8_t c1 = (color_val >> 8) & 0xFF;
    uint8_t c2 = (color_val >> 16) & 0xFF;
    for (uint32_t y = 0; y < height; y++) {
      uint8_t *line = fb + y * bytes_per_line;
      for (uint32_t x = 0; x < width; x++) {
        uint32_t idx = x * 3;
        line[idx] = c0;
        line[idx + 1] = c1;
        line[idx + 2] = c2;
      }
    }
  } else if (bpp == 2) {
    uint16_t val16 = (uint16_t)color_val;
    for (uint32_t y = 0; y < height; y++) {
      uint16_t *line = (uint16_t *)(fb + y * bytes_per_line);
      for (uint32_t x = 0; x < width; x++) {
        line[x] = val16;
      }
    }
  }
}

static int bios_checkkey(void) {
  struct realmode_regs int_regs = {0,  0,  0,  -1, 0,  0,          0,  0x0100,
                                   -1, -1, -1, -1, -1, 0xFFFF16CD, -1, -1};
  realmode_run((long)&int_regs);
  if ((int_regs.eflags & (1 << 6)) == 0) {
    return 1;
  }
  return 0;
}

static int bios_getkey(void) {
  struct realmode_regs int_regs = {0,  0,  0,  -1, 0,  0,          0,  0x0000,
                                   -1, -1, -1, -1, -1, 0xFFFF16CD, -1, -1};
  realmode_run((long)&int_regs);
  uint8_t ascii = int_regs.eax & 0xFF;
  uint8_t scan = (int_regs.eax >> 8) & 0xFF;
  if (ascii != 0) {
    return ascii;
  }
  return scan << 8;
}

#endif
