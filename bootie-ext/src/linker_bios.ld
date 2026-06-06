ENTRY(main)
SECTIONS
{
    . = 0;
    .text : {
        *screen_test_entry.o(.text)
        *(.text.startup)
        *(.text)
        *(.text.*)
        etext = .;
        _etext = .;
    }
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }
    .data.rel.ro : {
        *(.data.rel.ro)
        *(.data.rel.ro.*)
    }
    .got : {
        *(.got)
        *(.got.plt)
    }
    .data : {
        *(.data)
        *(.data.*)
        edata = .;
        _edata = .;
    }
    .grub_sig : {
        LONG(0x03051805);
        LONG(0xBCBAA7BA);
    }
    __BSS_START = .;
    __bss_start = .;
    .bss : {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }
    end = .;
    _end = .;
    __BSS_END = .;
}
