#ifndef BOOTIE_DEBUG_H
#define BOOTIE_DEBUG_H

/*
 * === Debug output to physical memory (bootie-debug.h) ===
 *
 * These helpers write null-terminated strings to a fixed physical address,
 * readable live from a QEMU debugger (GDB/LLDB).  The default address
 * 0x6000 is safe under GRUB4DOS (in the free area below 0x7C00).
 *
 * USAGE:
 *   1. Include this header:
 *         #include <bootie-debug.h>
 *
 *   2. Write debug messages anywhere in your code:
 *         dbg_reset();
 *         dbg_write("hello world\n");
 *         dbg_hex(some_value);
 *         dbg_putchar('!');
 *
 *   3. Launch QEMU with the GDB stub:
 *         qemu-system-x86_64 -s -m 2G -drive ...
 *
 *   4. Connect with your debugger and read the string:
 *
 *      LLDB:
 *         (lldb) gdb-remote 1234
 *         (lldb) x/s 0x6000
 *
 *      GDB:
 *         (gdb) target remote :1234
 *         (gdb) x/s 0x6000
 *
 * LIMITATIONS:
 *   - Writes raw bytes to physical memory; no buffer-overflow protection.
 *   - Default buffer is 256 bytes (0x6000 – 0x60FF).  Change DBG_ADDR
 *     and/or add a length guard if you need more.
 *   - Works only when a debugger is attached – zero effect on normal
 *     operation, but the writes are still performed.
 */

#ifndef DBG_ADDR
#define DBG_ADDR ((volatile unsigned char *)0x6000)
#endif

#ifndef DBG_CAP
#define DBG_CAP 256
#endif

/* Reset the debug buffer (writes a null terminator at the start). */
static inline void dbg_reset(void) {
    DBG_ADDR[0] = '\0';
}

/* Write a single character to the debug buffer (no overflow guard). */
static inline void dbg_putchar(int c) {
    volatile unsigned char *p = DBG_ADDR;
    while (*p) p++;
    *p++ = (unsigned char)c;
    *p = '\0';
}

/* Write a null-terminated string to the debug buffer. */
static inline void dbg_write(const char *s) {
    volatile unsigned char *p = DBG_ADDR;
    while (*p) p++;
    while (*s)
        *p++ = *s++;
    *p = '\0';
}

/* Write an unsigned long as hexadecimal (e.g. 0x1A3F → "1A3F"). */
static inline void dbg_hex(unsigned long v) {
    volatile unsigned char *p = DBG_ADDR;
    while (*p) p++;
    static const char hex[] = "0123456789ABCDEF";
    char buf[12];
    int i, j = 0;
    for (i = 28; i >= 0; i -= 4)
        buf[j++] = hex[(v >> i) & 0xF];
    buf[j] = '\0';
    char *s = buf;
    while (*s == '0' && s[1]) s++;
    while (*s) *p++ = *s++;
    *p = '\0';
}

#endif /* BOOTIE_DEBUG_H */
