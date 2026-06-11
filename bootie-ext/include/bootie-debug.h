#ifndef BOOTIE_DEBUG_H
#define BOOTIE_DEBUG_H

/*
 * === QEMU isa-debugcon debug output (bootie-debug.h) ===
 *
 * Writes debug output to QEMU's isa-debugcon device at I/O port 0xE9.
 *
 * USAGE:
 *   1. #include <bootie-debug.h>
 *   2. bt_dbg_printf("booted at %d ms", ticks);
 *
 * QEMU INVOCATION:
 *   qemu-system-x86_64 ... -debugcon stdio
 *   qemu-system-x86_64 ... -debugcon file:debug.log
 */

static void bt_dbg_putc(char c) {
    __asm__ volatile("outb %0, %1" : : "a"((unsigned char)c), "Nd"((unsigned short)0xE9));
}

static void bt_dbg_printf(const char *fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);

    while (*fmt) {
        if (*fmt != '%') {
            bt_dbg_putc(*fmt++);
            continue;
        }
        fmt++;
        if (*fmt == 'l') fmt++;

        switch (*fmt) {
        case 's': {
            const char *s = __builtin_va_arg(args, const char *);
            if (!s) s = "(null)";
            while (*s) bt_dbg_putc(*s++);
            break;
        }
        case 'd':
        case 'i': {
            long v = __builtin_va_arg(args, long);
            if (v < 0) { bt_dbg_putc('-'); v = -v; }
            goto print_ulong;
        case 'u':
            v = (long)__builtin_va_arg(args, unsigned long);
print_ulong: {
            char buf[24];
            int i = sizeof(buf) - 1;
            buf[i] = '\0';
            if (v == 0) buf[--i] = '0';
            else while (v && i > 0) {
                int d = (int)(v % 10);
                buf[--i] = (char)('0' + d);
                v /= 10;
            }
            while (buf[i]) bt_dbg_putc(buf[i++]);
            break;
        }}
        break;
        case 'x':
        case 'X': {
            unsigned long v = __builtin_va_arg(args, unsigned long);
            char buf[24];
            int i = sizeof(buf) - 1;
            buf[i] = '\0';
            if (v == 0) buf[--i] = '0';
            else while (v && i > 0) {
                int d = (int)(v & 0xf);
                buf[--i] = (char)(d < 10 ? '0' + d : 'A' + d - 10);
                v >>= 4;
            }
            while (buf[i]) bt_dbg_putc(buf[i++]);
            break;
        }
        case 'c': {
            int c = __builtin_va_arg(args, int);
            bt_dbg_putc((char)c);
            break;
        }
        case 'p': {
            void *p = __builtin_va_arg(args, void *);
            unsigned long v = (unsigned long)p;
            bt_dbg_putc('0');
            bt_dbg_putc('x');
            char buf[24];
            int i = sizeof(buf) - 1;
            buf[i] = '\0';
            if (v == 0) buf[--i] = '0';
            else while (v && i > 0) {
                int d = (int)(v & 0xf);
                buf[--i] = (char)(d < 10 ? '0' + d : 'A' + d - 10);
                v >>= 4;
            }
            while (buf[i]) bt_dbg_putc(buf[i++]);
            break;
        }
        case '%':
            bt_dbg_putc('%');
            break;
        default:
            bt_dbg_putc('%');
            if (*fmt) bt_dbg_putc(*fmt);
            break;
        }
        if (*fmt) fmt++;
    }

    __builtin_va_end(args);
}

#endif /* BOOTIE_DEBUG_H */
