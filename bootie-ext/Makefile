CC      := x86_64-elf-gcc
OBJCOPY := x86_64-elf-objcopy
RM      := rm -f
MKDIR   := mkdir -p

CFLAGS_BIOS := -O2 -Wl,--build-id=none -m32 -mno-sse -nostdlib \
               -fno-zero-initialized-in-bss -fno-function-cse \
               -fshort-wchar -fno-jump-tables -fno-tree-loop-distribute-patterns \
               -Wl,-N -Wl,-m,elf_i386 -Wl,-Ttext -Wl,0 -fPIE

CFLAGS_UEFI := -O2 -Wl,--build-id=none -m64 -mno-sse -nostdlib \
               -fno-zero-initialized-in-bss -fno-function-cse \
               -fshort-wchar -fno-jump-tables -fno-tree-loop-distribute-patterns \
               -Wl,-N -fPIE

# Collect all top-level C sources, excluding screen_test helper files
SOURCES := $(filter-out src/screen_test_sig.c src/screen_test_entry.c,$(wildcard src/*.c))
NAMES   := $(patsubst src/%.c,%,$(SOURCES))

BIOS_TARGETS := $(patsubst %,mod/bios/%,$(NAMES))
UEFI_TARGETS := $(patsubst %,mod/uefi/%,$(filter-out screen_test,$(NAMES)))

.PHONY: all bios uefi clean

all: bios uefi

bios: $(BIOS_TARGETS)

uefi: $(UEFI_TARGETS)

# ------------------------------------------------------------------
# Generic BIOS module build (all targets except screen_test)
# ------------------------------------------------------------------
mod/bios/%: src/%.c
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS_BIOS) -Iinclude $< -o $@.o
	$(OBJCOPY) -R .note.gnu.property -R .note.gnu.build-id -O binary $@.o $@
	$(RM) $@.o

# ------------------------------------------------------------------
# Generic UEFI module build
# ------------------------------------------------------------------
mod/uefi/%: src/%.c
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS_UEFI) -Iinclude -Iinclude/uefi $< -o $@.o
	$(OBJCOPY) -R .note.gnu.property -R .note.gnu.build-id -O binary $@.o $@
	$(RM) $@.o

# ------------------------------------------------------------------
# screen_test (BIOS only) - pure C + VBE, no LVGL
# ------------------------------------------------------------------
mod/bios/screen_test: src/screen_test_entry.c src/screen_test.c src/screen_test_sig.c
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS_BIOS) -Wl,-T,src/linker_bios.ld -Iinclude \
	      src/screen_test_entry.c src/screen_test.c src/screen_test_sig.c \
	      -o $@.o
	$(OBJCOPY) -R .note.gnu.property -R .note.gnu.build-id -O binary $@.o $@

# ------------------------------------------------------------------
clean:
	rm -rf mod
