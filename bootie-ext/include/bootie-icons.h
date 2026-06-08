#ifndef BOOTIE_ICONS_H
#define BOOTIE_ICONS_H

#include <stdint.h>

/* 16x16 pixel icons.
   Bit 0 = leftmost pixel, 1 = opaque, 0 = transparent.
   Rows are packed as little-endian uint16_t values. */

/* Solid disc (ISO/CD-ROM files) */
static const uint16_t ICON_DISC[16] = {
    0x0000,
    0x07e0,
    0x1f38,
    0x3f0c,
    0x3fc4,
    0x7e66,
    0x7c32,
    0x7992,
    0x799e,
    0x7c3e,
    0x7e7e,
    0x3ffc,
    0x3ffc,
    0x1ff8,
    0x07e0,
    0x0000,
};

/* Solid folder (directories) */
static const uint16_t ICON_FOLDER[16] = {
    0x0000,
    0x007c,
    0x00fe,
    0x3ffe,
    0x7ffe,
    0x7ffe,
    0x7ffe,
    0x7ffe,
    0x7ffe,
    0x7ffe,
    0x7ffe,
    0x7ffe,
    0x7ffe,
    0x7ffe,
    0x3ffc,
    0x0000,
};

/* Regular file page (non-bootable files) */
static const uint16_t ICON_FILE[16] = {
    0x0000,
    0x0ff0,
    0x3810,
    0x3810,
    0x7810,
    0x4010,
    0x4310,
    0x4310,
    0x4f00,
    0x4ffe,
    0x4700,
    0x4310,
    0x4110,
    0x4010,
    0x7ff0,
    0x0000,
};

#endif /* BOOTIE_ICONS_H */
