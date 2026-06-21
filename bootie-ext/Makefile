# Auto-detect cross-compiler (x86_64-elf-*) or fall back to native
CROSS_PREFIX := x86_64-elf-
CC      := $(or $(shell command -v $(CROSS_PREFIX)gcc 2>/dev/null),gcc)
OBJCOPY := $(or $(shell command -v $(CROSS_PREFIX)objcopy 2>/dev/null),objcopy)
RM      := rm -f
MKDIR   := mkdir -p

# TrueType font to embed (path to a .ttf file).
# Converted to a C array and compiled into every module.
TTF_FONT ?= fonts/SpaceGrotesk-Regular.ttf
FONT_HEADER := include/bootie-fonts.h

# Icon pack: PNGs are converted to a C header at build time.
ICON_DIR  := icons
ICON_HEADER := include/bootie-icons.h

# When using a native Linux gcc, -fPIE implies -pie which makes the linker
# generate PT_PHDR / PT_GNU_RELRO program headers that the custom linker
# script does not cover.  Suppress them with --no-dynamic-linker and norelro.
CFLAGS_BIOS := -O2 -Wl,--build-id=none -m32 -mno-sse -nostdlib \
               -fno-tree-loop-distribute-patterns \
               -Wl,-N -Wl,-m,elf_i386 -Wl,-T,ld/linker_bios.ld -fPIE \
               -Wl,--no-dynamic-linker -Wl,-z,norelro -Wl,--no-warn-rwx-segments \
               -Ilibs/schrift -Ilibs/lodepng -Ilibs/stb -MD -MP

# -mno-red-zone: required for x86_64 bare-metal because async callbacks (UEFI
# timer interrupts, etc.) run on the same stack and would corrupt red-zone data.
# -fshort-wchar: UEFI uses 16-bit wchar_t internally (MS ABI convention).
# -fno-asynchronous-unwind-tables: no need for .eh_frame in a flat binary.
CFLAGS_UEFI := -O2 -Wl,--build-id=none -m64 -mno-red-zone -nostdlib \
               -fshort-wchar -fno-tree-loop-distribute-patterns \
               -fno-asynchronous-unwind-tables -Wl,-N -Wl,-T,ld/linker_uefi.ld -fPIE \
               -Wl,--no-dynamic-linker -Wl,-z,norelro -Wl,--no-warn-rwx-segments \
               -Ilibs/schrift -Ilibs/lodepng -Ilibs/stb -MD -MP

# Collect all top-level C sources
SOURCES := $(wildcard src/*.c)
NAMES   := $(patsubst src/%.c,%,$(SOURCES))

BIOS_TARGETS := $(patsubst %,mod/bios/%,$(NAMES))
UEFI_TARGETS := $(patsubst %,mod/uefi/%,$(NAMES))

.PHONY: all bios uefi clean font-header icon-header

all: bios uefi

bios: $(BIOS_TARGETS)

uefi: $(UEFI_TARGETS)

# Auto-dependency tracking: -MT $@ so generated .d rules target the binary
DEPFLAGS = -MT $@

# Pull in dependency files (silently skip if none exist yet)
-include $(wildcard mod/bios/*.d mod/uefi/*.d)

# ------------------------------------------------------------------
# Generate font header from TTF
# ------------------------------------------------------------------
font-header: $(FONT_HEADER)

$(FONT_HEADER): $(TTF_FONT) scripts/ttf_to_c.py
	python3 scripts/ttf_to_c.py "$(TTF_FONT)" "$(FONT_HEADER)"

# ------------------------------------------------------------------
# Generate icon header from PNGs in $(ICON_DIR)
# ------------------------------------------------------------------
$(ICON_HEADER): $(wildcard $(ICON_DIR)/*.png) scripts/icons_to_c.py
	python3 scripts/icons_to_c.py "$(ICON_DIR)" "$(ICON_HEADER)"

# ------------------------------------------------------------------
# Generic BIOS module build
# ------------------------------------------------------------------
mod/bios/%: src/%.c $(FONT_HEADER) $(ICON_HEADER)
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS_BIOS) $(DEPFLAGS) -Iinclude $< -o $@.elf
	$(OBJCOPY) -R .note.gnu.property -R .note.gnu.build-id -O binary $@.elf $@
	$(RM) $@.elf

# ------------------------------------------------------------------
# Generic UEFI module build
# ------------------------------------------------------------------
mod/uefi/%: src/%.c $(FONT_HEADER) $(ICON_HEADER)
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS_UEFI) $(DEPFLAGS) -Iinclude -Iinclude/uefi $< -o $@.elf
	$(OBJCOPY) -R .note.gnu.property -R .note.gnu.build-id -O binary $@.elf $@
	$(RM) $@.elf

# ------------------------------------------------------------------
clean:
	rm -rf mod $(FONT_HEADER) $(ICON_HEADER)
