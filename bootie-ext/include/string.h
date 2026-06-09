#ifndef _STRING_H
#define _STRING_H
/* Minimal stub for bare-metal x86.
   GRUB4DOS provides memset/memmove/memcmp/strcmp as syscall macros
   (defined in bootie.h via bios/uefi grub4dos.h).  memcpy is not a
   GRUB4DOS syscall — map it to memmove (safe for stb_ds usage where
   source/dest never overlap). */
#include <stddef.h>
#define memcpy(d,s,n) memmove(d,s,n)
#endif
