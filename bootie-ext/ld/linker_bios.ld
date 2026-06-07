ENTRY(main)
PROVIDE(_start = main);
SECTIONS
{
    . = 0;
    .text : {
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
    .grub_header : {
        QUAD(0x534f443442555247); /* "GRUB4DOS" */
        LONG(main);
        LONG(ADDR(.text));
        LONG(etext);
        LONG(ADDR(.data));
        LONG(edata);
        LONG(__bss_start);
        LONG(ADDR(.bss));
        LONG(end);
        QUAD(0x646e655f6e69616d); /* "main_end" */
        LONG(0);
        LONG(0);
        LONG(0);
        LONG(0);
    }
    .data : {
        *(.data)
        *(.data.*)
        edata = .;
        _edata = .;
    }
    .grub_sig : {
        QUAD(0xBCBAA7BA03051805);
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
