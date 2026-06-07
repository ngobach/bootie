#define __BSS_END end
#define __BSS_START __bss_start

#define PROG_PSP (char *)(((int)&__BSS_END + 16) & ~0x0F)
extern int __BSS_END;
extern int __BSS_START;
