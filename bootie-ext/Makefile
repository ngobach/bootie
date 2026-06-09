# Auto-detect cross-compiler (x86_64-elf-*) or fall back to native
CROSS_PREFIX := x86_64-elf-
CC      := $(or $(shell command -v $(CROSS_PREFIX)gcc 2>/dev/null),gcc)
OBJCOPY := $(or $(shell command -v $(CROSS_PREFIX)objcopy 2>/dev/null),objcopy)
RM      := rm -f
MKDIR   := mkdir -p

# Optional TrueType font to embed (path to a .ttf file).
# When set, the font is converted to a C array and compiled into every module.
TTF_FONT ?= fonts/SpaceGrotesk-Regular.ttf
FONT_HEADER := include/font-ttf.h

# Icon pack: 16x16 PNGs are converted to a C header at build time.
ICON_DIR  := icons
ICON_HEADER := include/bootie-icons.h

# When using a native Linux gcc, -fPIE implies -pie which makes the linker
# generate PT_PHDR / PT_GNU_RELRO program headers that the custom linker
# script does not cover.  Suppress them with --no-dynamic-linker and norelro.
CFLAGS_BIOS := -O2 -Wl,--build-id=none -m32 -mno-sse -nostdlib \
               -fno-tree-loop-distribute-patterns \
               -Wl,-N -Wl,-m,elf_i386 -Wl,-T,ld/linker_bios.ld -fPIE \
               -Wl,--no-dynamic-linker -Wl,-z,norelro -Wl,--no-warn-rwx-segments \
               -Ilibs/schrift -Ilibs/lodepng

CFLAGS_UEFI := -O2 -Wl,--build-id=none -m64 -mno-red-zone -nostdlib \
               -fshort-wchar -fno-tree-loop-distribute-patterns \
               -fno-asynchronous-unwind-tables -Wl,-N -Wl,-T,ld/linker_uefi.ld -fPIE \
               -Wl,--no-dynamic-linker -Wl,-z,norelro -Wl,--no-warn-rwx-segments \
               -Ilibs/schrift -Ilibs/lodepng

# Collect all top-level C sources
SOURCES := $(wildcard src/*.c)
NAMES   := $(patsubst src/%.c,%,$(SOURCES))

BIOS_TARGETS := $(patsubst %,mod/bios/%,$(NAMES))
UEFI_TARGETS := $(patsubst %,mod/uefi/%,$(NAMES))

.PHONY: all bios uefi clean font-header icon-header

all: bios uefi

bios: $(BIOS_TARGETS)

uefi: $(UEFI_TARGETS)

# ------------------------------------------------------------------
# Generate font header from TTF
# ------------------------------------------------------------------
font-header: $(FONT_HEADER)

ifneq ($(wildcard $(TTF_FONT)),)
$(FONT_HEADER): $(TTF_FONT) scripts/ttf_to_c.py
	python3 scripts/ttf_to_c.py "$(TTF_FONT)" "$(FONT_HEADER)"
else
$(FONT_HEADER):
	@echo "No TTF font found at $(TTF_FONT); building without TrueType support."
	@echo "Set TTF_FONT=path/to/font.ttf to enable TrueType rendering."
endif

# ------------------------------------------------------------------
# Generate icon header from 16x16 RGBA PNGs in $(ICON_DIR)
# ------------------------------------------------------------------
$(ICON_HEADER): $(wildcard $(ICON_DIR)/*.png) scripts/icons_to_c.py
	python3 scripts/icons_to_c.py "$(ICON_DIR)" "$(ICON_HEADER)"

# ------------------------------------------------------------------
# Generic BIOS module build
# ------------------------------------------------------------------
mod/bios/%: src/%.c $(FONT_HEADER) $(ICON_HEADER)
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS_BIOS) -Iinclude $< -o $@.o
	$(OBJCOPY) -R .note.gnu.property -R .note.gnu.build-id -O binary $@.o $@
	$(RM) $@.o

# ------------------------------------------------------------------
# Generic UEFI module build
# ------------------------------------------------------------------
mod/uefi/%: src/%.c $(FONT_HEADER) $(ICON_HEADER)
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS_UEFI) -Iinclude -Iinclude/uefi $< -o $@.o
	$(OBJCOPY) -R .note.gnu.property -R .note.gnu.build-id -O binary $@.o $@
	$(RM) $@.o

# ------------------------------------------------------------------
clean:
	rm -rf mod $(FONT_HEADER) $(ICON_HEADER)
