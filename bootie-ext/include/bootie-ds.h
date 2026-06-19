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
   On UEFI, uefi_malloc stores the user-requested size in a hidden
   header, so we can copy exactly the old bytes (fixing the heap
   over-read that corrupted GRUB4EFI's internal heap).
   On BIOS, GRUB4DOS malloc does not track sizes, so we copy the full
   new_size — this over-reads but the memory is still accessible. */
static inline void *bt_realloc(void *ptr, size_t size) {
    if (!ptr) return malloc(size);
    if (!size) { free(ptr); return NULL; }
    void *newp = malloc(size);
    if (newp) {
#if !defined(__i386__)
        grub_size_t old_sz = *(grub_size_t *)((char *)ptr - UEFI_POOL_HDR);
        grub_size_t copy = old_sz < (grub_size_t)size ? old_sz : (grub_size_t)size;
        memmove(newp, ptr, copy);
#else
        memmove(newp, ptr, size);
#endif
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
