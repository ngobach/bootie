/* Linker symbols defining the BSS segment boundaries.
   In 32-bit BIOS mode, they are declared as standard extern variables. Their
   addresses are retrieved using the address-of operator (&__BSS_START) in
   bootprog.h. */
extern int __BSS_END;
extern int __BSS_START;
