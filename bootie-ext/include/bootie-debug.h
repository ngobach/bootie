#ifndef BOOTIE_DEBUG_H
#define BOOTIE_DEBUG_H

/*
 * === Heap-allocated debug log (bootie-debug.h) ===
 *
 * The heap buffer pointer is stored at physical address 0x6000 so a
 * debugger can find it instantly — no memory scanning needed:
 *
 *   - BIOS: 0x6000 is in free conventional memory (< 0x7C00).
 *   - UEFI:  the page at 0x6000 is explicitly allocated from UEFI boot
 *            services (AllocatePages) before writing, making the write
 *            both safe and identity-mapped.
 *
 * USAGE:
 *   1. #include <bootie.h>  (provides malloc)
 *   2. #include <bootie-debug.h>
 *   3. bt_dbg_init(4096);
 *   4. bt_dbg_printf("booted at %d ms\n", ticks);
 *
 * DISCOVERY (from the debugger):
 *
 *   Read the pointer at 0x6000, then read the string:
 *
 *     GDB:
 *       (gdb) x/gx 0x6000       -> 0x7bceddc0
 *       (gdb) x/s *0x6000       -> full log
 *
 *     LLDB:
 *       (lldb) x/gx 0x6000      -> 0x7bceddc0
 *       (lldb) x/s 0x7bceddc0   -> full log
 *
 *   Or load the unstripped .o and use the global symbol:
 *     (lldb) p bt_dbg_buffer
 */

/* Non-static global so the symbol survives in the ELF .o file for
   debuggers that can load symbols.  Each module gets its own copy. */
char *bt_dbg_buffer;

static int bt_dbg_off;
static int bt_dbg_cap;

/* ------------------------------------------------------------------ */
/*  Pointer hub at physical 0x6000 (BIOS) or allocated (UEFI)          */
/* ------------------------------------------------------------------ */

static void bt_dbg_store_ptr(void) {
#if defined(__i386__)
    *(volatile char **)0x6000 = bt_dbg_buffer;
#else
    efi_system_table_t *st = grub_efi_system_table;
    if (st && st->boot_services) {
        efi_physical_address_t addr = 0x6000;
        efi_status_t status = st->boot_services->allocate_pages(
            EFI_ALLOCATE_ADDRESS,
            EFI_BOOT_SERVICES_DATA,
            1,
            &addr);
        if (status == 0)
            *(volatile char **)0x6000 = bt_dbg_buffer;
    }
#endif
}

/* ------------------------------------------------------------------ */
/*  Public API                                                         */
/* ------------------------------------------------------------------ */

static void bt_dbg_init(int capacity) {
    if (capacity < 4096) capacity = 4096;
    bt_dbg_buffer = (char *)malloc((unsigned int)capacity);
    if (bt_dbg_buffer) {
        bt_dbg_cap = capacity;
        bt_dbg_off = 0;
        bt_dbg_buffer[0] = '\0';
        bt_dbg_store_ptr();
    }
}

static void bt_dbg_reset(void) {
    bt_dbg_off = 0;
    if (bt_dbg_buffer && bt_dbg_cap > 0)
        bt_dbg_buffer[0] = '\0';
}

static void bt_dbg_printf(const char *fmt, ...) {
    if (!bt_dbg_buffer) return;

    __builtin_va_list args;
    __builtin_va_start(args, fmt);

    while (*fmt && bt_dbg_off < bt_dbg_cap - 1) {
        if (*fmt != '%') {
            bt_dbg_buffer[bt_dbg_off++] = *fmt++;
            continue;
        }
        fmt++;
        if (*fmt == 'l') fmt++;

        switch (*fmt) {
        case 's': {
            const char *s = __builtin_va_arg(args, const char *);
            if (!s) s = "(null)";
            while (*s && bt_dbg_off < bt_dbg_cap - 1)
                bt_dbg_buffer[bt_dbg_off++] = *s++;
            break;
        }
        case 'd':
        case 'i': {
            long v = __builtin_va_arg(args, long);
            if (v < 0) {
                bt_dbg_buffer[bt_dbg_off++] = '-';
                v = -v;
            }
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
            while (buf[i] && bt_dbg_off < bt_dbg_cap - 1)
                bt_dbg_buffer[bt_dbg_off++] = buf[i++];
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
            while (buf[i] && bt_dbg_off < bt_dbg_cap - 1)
                bt_dbg_buffer[bt_dbg_off++] = buf[i++];
            break;
        }
        case 'c': {
            int c = __builtin_va_arg(args, int);
            bt_dbg_buffer[bt_dbg_off++] = (char)c;
            break;
        }
        case 'p': {
            void *p = __builtin_va_arg(args, void *);
            unsigned long v = (unsigned long)p;
            bt_dbg_buffer[bt_dbg_off++] = '0';
            bt_dbg_buffer[bt_dbg_off++] = 'x';
            char buf[24];
            int i = sizeof(buf) - 1;
            buf[i] = '\0';
            if (v == 0) buf[--i] = '0';
            else while (v && i > 0) {
                int d = (int)(v & 0xf);
                buf[--i] = (char)(d < 10 ? '0' + d : 'A' + d - 10);
                v >>= 4;
            }
            while (buf[i] && bt_dbg_off < bt_dbg_cap - 1)
                bt_dbg_buffer[bt_dbg_off++] = buf[i++];
            break;
        }
        case '%':
            bt_dbg_buffer[bt_dbg_off++] = '%';
            break;
        default:
            bt_dbg_buffer[bt_dbg_off++] = '%';
            if (*fmt && bt_dbg_off < bt_dbg_cap - 1)
                bt_dbg_buffer[bt_dbg_off++] = *fmt;
            break;
        }
        if (*fmt) fmt++;
    }

    __builtin_va_end(args);
    bt_dbg_buffer[bt_dbg_off] = '\0';
}

#endif /* BOOTIE_DEBUG_H */
