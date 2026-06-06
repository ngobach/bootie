/* screen_test_sig.c
 * Contains the _start symbol for screen_test.
 * Linked at the very end of the executable.
 */
#define NO_GRUB_HEADER
#define NO_GRUB_SIGNATURE
#include <bootprog.h>

asm(".text");
asm(".globl _start");
asm("_start:");
