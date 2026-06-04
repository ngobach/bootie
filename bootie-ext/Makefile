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

.PHONY: all bios uefi clean prepare

all: prepare bios uefi

prepare:
	$(MKDIR) bin

bios: bin/hello_bios

uefi: bin/hello_uefi

bin/hello_bios: src/main.c
	$(CC) $(CFLAGS_BIOS) -Iinclude $< -o main_bios.o
	$(OBJCOPY) -R .note.gnu.property -R .note.gnu.build-id -O binary main_bios.o $@
	$(RM) main_bios.o

bin/hello_uefi: src/main.c
	$(CC) $(CFLAGS_UEFI) -Iinclude -Iinclude/uefi $< -o main_uefi.o
	$(OBJCOPY) -R .note.gnu.property -R .note.gnu.build-id -O binary main_uefi.o $@
	$(RM) main_uefi.o

clean:
	$(RM) main_bios.o main_bios.mod main_uefi.o main_uefi.mod
	rm -rf bin
