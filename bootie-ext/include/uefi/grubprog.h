/* Linker symbols defining the BSS segment boundaries.
   We declare them as hidden character arrays so that GCC compiles position-independent,
   RIP-relative address calculations (using 'lea') instead of absolute address loads
   or GOT dereferencing. */
__attribute__((visibility("hidden"))) extern char __BSS_END[];
__attribute__((visibility("hidden"))) extern char __BSS_START[];
