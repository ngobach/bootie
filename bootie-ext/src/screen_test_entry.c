/*
 * screen_test_entry.c - Entry point for screen_test GRUB4DOS module (BIOS)
 *
 * Responsibilities:
 *   1. Zero-initialise the BSS section (flat binary does not carry BSS data)
 *   2. Call INIT_BOOT_MODULE() to set up the GRUB4DOS runtime environment
 *   3. Dispatch to screen_test_main()
 */
#define NO_GRUB_SIGNATURE
#include <bootprog.h>

extern int __BSS_START;
extern int __BSS_END;

extern int screen_test_main(char *arg, int flags);

int main(char *arg, int flags) {
    /* Zero-initialise BSS */
    char *p = (char *)&__BSS_START;
    char *e = (char *)&__BSS_END;
    while (p < e) *p++ = 0;

    INIT_BOOT_MODULE();
    return screen_test_main(arg, flags);
}
