#ifndef BOOTIE_DS_H
#define BOOTIE_DS_H

/* ------------------------------------------------------------------ */
/*  stb_ds wrapper — dynamic arrays & hash maps for bare-metal x86    */
/*                                                                     */
/*  Usage:                                                             */
/*    #include <bootie.h>                                              */
/*    #include <bootie-ds.h>                                           */
/*                                                                     */
/*    int *arr = NULL;                                                 */
/*    arrput(arr, 42);                                                 */
/*    arrlen(arr);  // 1                                               */
/*    arrfree(arr);                                                     */
/*                                                                     */
/*  Memory: GRUB4DOS malloc/free via STBDS_REALLOC/STBDS_FREE.         */
/*  No libc needed.                                                    */
/* ------------------------------------------------------------------ */

/* realloc shim — GRUB4DOS provides malloc/free/memmove but no realloc.
   The extra copy bytes are immediately overwritten by stb_ds
   initialization, so the over-read is harmless in practice.
   (Same pattern as lodepng_realloc in bootie-img.h.) */
static inline void *bt_realloc(void *ptr, size_t size) {
    if (!ptr) return malloc(size);
    if (!size) { free(ptr); return NULL; }
    void *newp = malloc(size);
    if (newp) {
        memmove(newp, ptr, size);
        free(ptr);
    }
    return newp;
}

#define STBDS_REALLOC(ctx,ptr,sz)  bt_realloc(ptr, sz)
#define STBDS_FREE(ctx,ptr)        free(ptr)
#define STBDS_ASSERT(x)            ((void)0)

/* stb_ds implementation uses memcpy internally (hash map key copy, etc.);
   GRUB4DOS provides memmove but not memcpy — redirect safely. */
#ifndef memcpy
#define memcpy(d,s,n) memmove(d,s,n)
#endif

#define STB_DS_IMPLEMENTATION
#include <stb_ds.h>

#endif /* BOOTIE_DS_H */
