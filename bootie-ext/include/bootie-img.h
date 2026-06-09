#ifndef BOOTIE_IMG_H
#define BOOTIE_IMG_H

#include <bootie-gfx.h>

/* Disable lodepng features we don't need to save binary size */
#define LODEPNG_NO_COMPILE_ENCODER
#define LODEPNG_NO_COMPILE_DISK
#define LODEPNG_NO_COMPILE_CPP
#define LODEPNG_NO_COMPILE_ANCILLARY_CHUNKS
/* Disable built-in allocators: we provide them below using GRUB4DOS macros */
#define LODEPNG_NO_COMPILE_ALLOCATORS

/*
  Push/pop the UEFI grub4dos.h `image` macro so it doesn't expand
  inside the lodepng code, then restore it afterwards.
*/
#pragma push_macro("image")
#undef image
#define image img

#include "../libs/lodepng/lodepng.c"

#pragma pop_macro("image")

/* Allocator functions used internally by lodepng.
   The GRUB4DOS environment provides malloc/free (SYSFUN 50/51) and
   memmove (SYSFUN 23) but not realloc, so we implement it here. */
void *lodepng_malloc(size_t size) { return malloc(size); }
void lodepng_free(void *ptr) { free(ptr); }
void *lodepng_realloc(void *ptr, size_t new_size) {
    if (!ptr) return lodepng_malloc(new_size);
    void *new_ptr = lodepng_malloc(new_size);
    if (new_ptr) {
        memmove(new_ptr, ptr, new_size);
        lodepng_free(ptr);
    }
    return new_ptr;
}

/* Decode PNG from memory to 32-bit RGBA pixels into a gfx_image.
   Returns 0 on success, non-zero error code on failure.
   Free the image pixels with gfx_png_free() when done. */
static inline int gfx_png_decode(const unsigned char *data, size_t data_size,
                                  struct gfx_image *out) {
    return lodepng_decode32(&out->pixels, &out->w, &out->h, data, data_size);
}

/* Free pixel buffer allocated by gfx_png_decode() */
static inline void gfx_png_free(struct gfx_image *img) {
    free(img->pixels);
    img->pixels = 0;
}

/* Blit 32-bit RGBA pixels to framebuffer at (x, y).
   Pixels with alpha > 128 are drawn; others are skipped. */
static inline void gfx_draw_image(struct gfx *ctx, int x, int y,
                                   const unsigned char *pixels,
                                   unsigned w, unsigned h) {
    for (unsigned row = 0; row < h; row++) {
        for (unsigned col = 0; col < w; col++) {
            const unsigned char *p = pixels + (row * w + col) * 4;
            if (p[3] > 128)
                fill_rect(ctx, x + col, y + row, 1, 1, p[0], p[1], p[2]);
        }
    }
}

#endif /* BOOTIE_IMG_H */
