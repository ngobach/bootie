CC      := x86_64-elf-gcc
OBJCOPY := x86_64-elf-objcopy
RM      := rm -f
MKDIR   := mkdir -p

GCCVERSIONGTEQ5 := $(shell expr `$(CC) -dumpversion | cut -f1 -d.` \>= 5)
GCCVERSIONGTEQ8 := $(shell expr `$(CC) -dumpversion | cut -f1 -d.` \>= 8)
GCCVERSIONGTEQ10 := $(shell expr `$(CC) -dumpversion | cut -f1 -d.` \< 10)

CFLAGS_BIOS := -Wl,--build-id=none -m32 -mno-sse -nostdlib \
               -fno-zero-initialized-in-bss -fno-function-cse \
               -fshort-wchar -fno-jump-tables -Wl,-N -Wl,-m,elf_i386 \
               -Wl,-Ttext -Wl,0 -fPIE

CFLAGS_UEFI := -Wl,--build-id=none -m64 -mno-sse -nostdlib \
               -fno-zero-initialized-in-bss -fno-function-cse \
               -fshort-wchar -fno-jump-tables -Wl,-N -fPIE

SOURCES := $(wildcard src/*.c)
NAMES := $(patsubst src/%.c,%,$(SOURCES))

BIOS_TARGETS := $(patsubst %,mod/bios/%,$(NAMES))
UEFI_TARGETS := $(patsubst %,mod/uefi/%,$(NAMES))

.PHONY: all bios uefi clean

all: bios uefi

bios: $(BIOS_TARGETS)

uefi: $(UEFI_TARGETS)

mod/bios/%: src/%.c
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS_BIOS) -Iinclude $< -o $@.o
	$(OBJCOPY) -R .note.gnu.property -R .note.gnu.build-id -O binary $@.o $@
	$(RM) $@.o

mod/uefi/%: src/%.c
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS_UEFI) -Iinclude -Iinclude/uefi $< -o $@.o
	$(OBJCOPY) -R .note.gnu.property -R .note.gnu.build-id -O binary $@.o $@
	$(RM) $@.o

clean:
	rm -rf mod
