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

/* Decode PNG from memory into a gfx_sprite (allocates RGBA pixels).
   Returns 0 on success, non-zero error code on failure.
   Free with gfx_sprite_destroy() when done. */
static inline int gfx_png_decode(const unsigned char *data, size_t data_size,
                                  struct gfx_sprite *out) {
    return lodepng_decode32(&out->pixels, &out->w, &out->h, data, data_size);
}

#endif /* BOOTIE_IMG_H */
